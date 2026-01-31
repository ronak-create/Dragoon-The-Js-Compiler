#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/semantic.h"

#define MAX_SCOPES 64
#define SEM_MAX_SYMBOLS 256

static int max_scope_depth = 0;


typedef struct {
    char name[50];
    int is_const;
    SemType type;
} SemanticSymbol;

typedef struct {
    int scope;
    int index;
} SymbolRef;



typedef struct {
    SemanticSymbol symbols[SEM_MAX_SYMBOLS];
    int count;
} Scope;

static Scope scopes[MAX_SCOPES];
static int scope_depth = -1;

static void extract_update_identifier(char *out, const char *expr) {
    int j = 0;
    for (int i = 0; expr[i]; i++) {
        if (isalpha(expr[i]) || expr[i] == '_')
            out[j++] = expr[i];
    }
    out[j] = '\0';
}


static const char *type_to_string(SemType t) {
    switch (t) {
        case TYPE_NUMBER: return "number";
        case TYPE_STRING: return "string";
        case TYPE_BOOLEAN: return "boolean";
        default: return "unknown";
    }
}

static SemType literal_type(const char *value) {
    if (!value) return TYPE_UNKNOWN;

    if (!strcmp(value, "true") || !strcmp(value, "false"))
        return TYPE_BOOLEAN;

    // numeric literals
    if (isdigit(value[0]) || 
        (value[0] == '-' && isdigit(value[1])) ||
        !strncmp(value, "0x", 2) ||
        !strncmp(value, "0b", 2))
        return TYPE_NUMBER;

    // everything else is string
    return TYPE_STRING;
}




/* ---------- Scope Management ---------- */

static void enter_scope() {
    scope_depth++;
    if (scope_depth > max_scope_depth)
        max_scope_depth = scope_depth;
    scopes[scope_depth].count = 0;
}


static void exit_scope() {
    scope_depth--;
}

static SymbolRef lookup_symbol(const char *name) {
    for (int i = scope_depth; i >= 0; i--) {
        for (int j = 0; j < scopes[i].count; j++) {
            if (strcmp(scopes[i].symbols[j].name, name) == 0)
                return (SymbolRef){ i, j };
        }
    }
    return (SymbolRef){ -1, -1 };
}



static void declare_symbol(const char *name, int is_const, SemType type) {
    Scope *scope = &scopes[scope_depth];

    for (int i = 0; i < scope->count; i++) {
        if (strcmp(scope->symbols[i].name, name) == 0) {
            printf("Semantic Error: redeclaration of '%s'\n", name);
            exit(1);
        }
    }

    strcpy(scope->symbols[scope->count].name, name);
    scope->symbols[scope->count].is_const = is_const;
    scope->symbols[scope->count].type = type;
    scope->count++;
}

static SemType analyze_expr(ASTNode *node) {
    if (!node) return TYPE_UNKNOWN;

    switch (node->type) {

    case AST_LITERAL:
        return literal_type(node->value);

    // case AST_IDENTIFIER: {
    //     SymbolRef t = lookup_symbol(node->value);
    //     if (t.scope == -1) {
    //         printf("Semantic Error: '%s' not declared\n", node->value);
    //         exit(1);
    //     }
    //     return scopes[t.scope].symbols[t.index].type;
    // }

    case AST_BINARY_OP: {
        SemType l = analyze_expr(node->left);
        SemType r = analyze_expr(node->right);

        if (strcmp(node->value, "+") == 0) {
            if (l == TYPE_STRING || r == TYPE_STRING)
                return TYPE_STRING;
            if (l == TYPE_NUMBER && r == TYPE_NUMBER)
                return TYPE_NUMBER;
        }

        if (l != TYPE_NUMBER || r != TYPE_NUMBER) {
            printf("Type Error: operator '%s' not valid for %s and %s\n",
                   node->value, type_to_string(l), type_to_string(r));
            exit(1);
        }
        return TYPE_NUMBER;
    }

    default:
        return TYPE_UNKNOWN;
    }
}



/* ---------- Semantic Walker ---------- */

static void analyze_node(ASTNode *node) {
    if (!node) return;

    switch (node->type) {

    case AST_POST_UPDATE:
    case AST_PRE_UPDATE: {
        char var[64];
        extract_update_identifier(var, node->value);

        SymbolRef ref = lookup_symbol(var);
        if (ref.scope == -1) {
            printf("Semantic Error: '%s' not declared\n", var);
            exit(1);
        }

        if (scopes[ref.scope].symbols[ref.index].type != TYPE_NUMBER) {
            printf("Type Error: update operator requires number, got %s\n",
                type_to_string(scopes[ref.scope].symbols[ref.index].type));
            exit(1);
        }

        if (scopes[ref.scope].symbols[ref.index].is_const) {
            printf("Semantic Error: cannot modify const '%s'\n", var);
            exit(1);
        }
        break;
    }



    case AST_BLOCK:
        enter_scope();
        for (int i = 0; i < node->body_size; i++)
            analyze_node(node->body[i]);
        exit_scope();
        break;
        
    case AST_ASSIGNMENT: {

        // Declaration
        if (node->left->type == AST_VAR_DECL) {
            SemType rhs_type = analyze_expr(node->right);
            declare_symbol(node->left->value, 0, rhs_type);
            return;
        }

        // Reassignment
        if (node->left->type == AST_IDENTIFIER) {
            SymbolRef idx = lookup_symbol(node->left->value);
            if (idx.scope == -1) {
                printf("Semantic Error: '%s' not declared\n", node->left->value);
                exit(1);
            }

            SemType rhs_type = analyze_expr(node->right);
            SemType lhs_type = scopes[idx.scope].symbols[idx.index].type;

            
                if (lhs_type == TYPE_UNKNOWN) {
                    scopes[idx.scope].symbols[idx.index].type = rhs_type;
                }
                else if (lhs_type != rhs_type) {
                    printf("Type Error: cannot assign %s to %s\n",
                    type_to_string(rhs_type), type_to_string(lhs_type));
                    exit(1);
                }
            }
        break;
    }

    case AST_IDENTIFIER:
        if (lookup_symbol(node->value).scope == -1) {
            printf("Semantic Error: '%s' is not declared\n", node->value);
            exit(1);
        }
        break;

    case AST_FOR_STMT:
        enter_scope();
        analyze_node(node->left); // init
        analyze_node(node->right->body[0]); // condition
        analyze_node(node->right->body[2]); // body
        analyze_node(node->right->body[1]); // update
        exit_scope();
        break;

    default:
        analyze_node(node->left);
        analyze_node(node->right);
        break;
    }
}

/* ---------- Public Entry ---------- */

void semantic_analyze(ASTNode *root) {
    enter_scope();
    analyze_node(root);
    exit_scope();
}

SemType semantic_get_type(const char *name) {
    for (int i = max_scope_depth; i >= 0; i--) {
        for (int j = 0; j < scopes[i].count; j++) {
            if (strcmp(scopes[i].symbols[j].name, name) == 0)
                return scopes[i].symbols[j].type;
        }
    }
    return TYPE_NUMBER;
}

