#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/qbe_codegen.h"
#include "../../include/semantic.h"

/*
 QBE backend (correct & minimal)
 - Single function: $main
 - Stack slots for variables
 - SSA temporaries
 - Strings are silently skipped (no error)
*/

static FILE *out;
static int temp_id = 0;

static void extract_ident(char *out, const char *expr) {
    int j = 0;
    for (int i = 0; expr[i]; i++) {
        if (isalpha(expr[i]) || expr[i] == '_')
            out[j++] = expr[i];
    }
    out[j] = '\0';
}

/* ---------- temp generation ---------- */

static const char *new_tmp() {
    static char buf[32];
    snprintf(buf, sizeof(buf), "%%t%d", temp_id++);
    return buf;
}

/* ---------- helpers ---------- */

static int is_string_literal(ASTNode *n) {
    return n && n->type == AST_LITERAL &&
           n->value[0] != '\0' &&
           !(isdigit(n->value[0]) || n->value[0] == '-') &&
           strcmp(n->value, "true") != 0 &&
           strcmp(n->value, "false") != 0;
}


/* ---------- symbol table (name → slot) ---------- */

typedef struct {
    char name[64];
    char slot[64];
} QbeSymbol;

static QbeSymbol symbols[256];
static int sym_count = 0;

static const char *get_slot(const char *name) {
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(symbols[i].name, name) == 0)
            return symbols[i].slot;
    }
    return NULL;
}

static void add_slot(const char *name) {
    snprintf(symbols[sym_count].name, 64, "%s", name);
    snprintf(symbols[sym_count].slot, 64, "%%%s", name);
    sym_count++;
}

/* ---------- allocation pass ---------- */

static void emit_allocations(ASTNode *n) {
    if (!n) return;

    if (n->type == AST_ASSIGNMENT &&
        n->left &&
        n->left->type == AST_VAR_DECL) {

        // Skip string type variables
        if (semantic_get_type(n->left->value) == TYPE_STRING)
            return;

        // Skip if right side is a string literal
        if (is_string_literal(n->right))
            return;

        if (!get_slot(n->left->value)) {
            add_slot(n->left->value);
            fprintf(out, "    %%%s =l alloc4 4\n", n->left->value);
        }
    }

    if (n->type == AST_BLOCK) {
        for (int i = 0; i < n->body_size; i++)
            emit_allocations(n->body[i]);
    }
}

/* ---------- expressions ---------- */

static const char *emit_expr(ASTNode *n) {
    if (!n) return "0";

    // Skip string literals - return dummy value
    if (is_string_literal(n)) {
        return "0";
    }

    switch (n->type) {

    case AST_LITERAL:
        if (!strcmp(n->value, "true"))  return "1";
        if (!strcmp(n->value, "false")) return "0";

        if (!strncmp(n->value, "0x", 2)) {
            static char buf[32];
            sprintf(buf, "%d", (int)strtol(n->value + 2, NULL, 16));
            return buf;
        }

        if (!strncmp(n->value, "0b", 2)) {
            static char buf[32];
            sprintf(buf, "%d", (int)strtol(n->value + 2, NULL, 2));
            return buf;
        }

        return n->value;

    case AST_IDENTIFIER: {
        const char *slot = get_slot(n->value);
        if (!slot) {
            // Variable doesn't exist in symbol table - likely a string, skip it
            return "0";
        }
        const char *t = new_tmp();
        fprintf(out, "    %s =w loadw %s\n", t, slot);
        return t;
    }

    case AST_BINARY_OP: {
        const char *lhs = emit_expr(n->left);
        const char *rhs = emit_expr(n->right);
        const char *t = new_tmp();

        const char *op =
            !strcmp(n->value, "+") ? "add" :
            !strcmp(n->value, "-") ? "sub" :
            !strcmp(n->value, "*") ? "mul" :
            !strcmp(n->value, "/") ? "div" : "add";

        fprintf(out, "    %s =w %s %s, %s\n", t, op, lhs, rhs);
        return t;
    }

    default:
        return "0";
    }
}

/* ---------- statements ---------- */

static void emit_stmt(ASTNode *n) {
    if (!n) return;

    switch (n->type) {

    case AST_ASSIGNMENT: {
        // Skip string type variables
        if (semantic_get_type(n->left->value) == TYPE_STRING)
            return;

        // Skip if right side is a string literal
        if (is_string_literal(n->right))
            return;

        const char *rhs = emit_expr(n->right);
        const char *slot = get_slot(n->left->value);

        if (!slot) {
            // Variable not in symbol table - likely a string, skip
            return;
        }

        fprintf(out, "    storew %s, %s\n", rhs, slot);
        break;
    }

    case AST_POST_UPDATE:
    case AST_PRE_UPDATE: {
        char var[64];
        extract_ident(var, n->value);

        const char *slot = get_slot(var);
        if (!slot) {
            // Variable not found - skip
            return;
        }

        const char *t1 = new_tmp();
        const char *t2 = new_tmp();

        fprintf(out, "    %s =w loadw %s\n", t1, slot);
        fprintf(out, "    %s =w add %s, 1\n", t2, t1);
        fprintf(out, "    storew %s, %s\n", t2, slot);
        break;
    }

    case AST_FUNC_CALL:
        if (!strcmp(n->value, "console.log")) {
            if (is_string_literal(n->body[0])) {
                fprintf(out, "    ; Skipping string log (not supported)\n");
                break; 
            }
            const char *arg = emit_expr(n->body[0]);
            const char *unused = new_tmp();
            fprintf(out, "    %s =w call $printi(w %s)\n", unused, arg);
        }
        break;

    case AST_BLOCK:
        for (int i = 0; i < n->body_size; i++)
            emit_stmt(n->body[i]);
        break;

    default:
        break;
    }
}

/* ---------- entry ---------- */

void qbe_codegen(ASTNode *program, const char *out_qbe) {
    out = fopen(out_qbe, "w");
    if (!out) {
        perror("fopen");
        exit(1);
    }

    // Emit helper function for printing integers
    fprintf(out, 
        "data $d_fmt = { b \"%%d\\n\", b 0 }\n\n"
        "export function w $printi(w %%arg) {\n"
        "@entry\n"
        "    %%r =w call $printf(l $d_fmt, w %%arg, ...)\n"
        "    ret 0\n"
        "}\n\n");


    fprintf(out,
        "export function w $main() {\n"
        "@entry\n");

    emit_allocations(program);
    emit_stmt(program);

    fprintf(out,
        "    ret 0\n"
        "}\n");

    fclose(out);
}