#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/codegen.h"
#include "../../include/semantic.h"

static FILE *out;

static int is_string_literal(ASTNode *n) {
    return n->type == AST_LITERAL &&
           n->value &&
           n->value[0] != '-' &&
           !isdigit(n->value[0]) &&
           strcmp(n->value, "true") != 0 &&
           strcmp(n->value, "false") != 0;
}

static void emit_declarations(ASTNode *root) {
    if (!root || root->type != AST_BLOCK) return;

    for (int i = 0; i < root->body_size; i++) {
        ASTNode *n = root->body[i];
        if (n->type == AST_ASSIGNMENT &&
            n->left->type == AST_VAR_DECL) {

            SemType t = semantic_get_type(n->left->value);

            if (t == TYPE_STRING)
                fprintf(out, "    char *%s;\n", n->left->value);
            else if (t == TYPE_BOOLEAN)
                fprintf(out, "    bool %s;\n", n->left->value);
            else
                fprintf(out, "    int %s;\n", n->left->value);
        }
        if (n->type == AST_FOR_STMT) {
            ASTNode *init = n->left;
            if (init && init->type == AST_ASSIGNMENT) {
                fprintf(out, "    int %s;\n", init->left->value);
            }
        }
    }
}

static SemType expr_type(ASTNode *n) {
    if (!n) return TYPE_NUMBER;

    switch (n->type) {
    case AST_LITERAL:
        if (!strcmp(n->value, "true") || !strcmp(n->value, "false"))
            return TYPE_BOOLEAN;
        if (is_string_literal(n))
            return TYPE_STRING;
        return TYPE_NUMBER;

    case AST_IDENTIFIER:
        return semantic_get_type(n->value);

    case AST_BINARY_OP:
        return expr_type(n->left);

    default:
        return TYPE_NUMBER;
    }
}




static void emit_expr(ASTNode *n) {
    if (!n) return;
    
    switch (n->type) {
        
        case AST_LITERAL:
            if (!strcmp(n->value, "true"))
                fprintf(out, "1");
            else if (!strcmp(n->value, "false"))
                fprintf(out, "0");
            else if (strncmp(n->value, "0b", 2) == 0)
                fprintf(out, "%d", (int)strtol(n->value + 2, NULL, 2));
            else if (strncmp(n->value, "0x", 2) == 0)
                fprintf(out, "%d", (int)strtol(n->value + 2, NULL, 16));
            else if (is_string_literal(n))
                fprintf(out, "\"%s\"", n->value);
            else
                fprintf(out, "%s", n->value);
            break;

        
    case AST_IDENTIFIER:
    fprintf(out, "%s", n->value);
    break;

    case AST_POST_UPDATE:
        fprintf(out, "%s", n->value); // "i++"
        break;

    case AST_PRE_UPDATE:
        fprintf(out, "%s", n->value); // "++i"
        break;

    
    case AST_BINARY_OP:
    fprintf(out, "(");
    emit_expr(n->left);
    
    if (strcmp(n->value, "===") == 0)
    fprintf(out, " == ");
    else
    fprintf(out, " %s ", n->value);
    
    emit_expr(n->right);
    fprintf(out, ")");
    break;
    
    default:
    break;
}
}

static void emit_for_part(ASTNode *n) {
    if (!n) return;

    if (n->type == AST_ASSIGNMENT) {
        fprintf(out, "%s = ", n->left->value);
        emit_expr(n->right);
    } else if (n->type == AST_POST_UPDATE ||
               n->type == AST_PRE_UPDATE) {
        fprintf(out, "%s", n->value);
    }
}


static void emit_stmt(ASTNode *n, int indent) {
    if (!n) return;

    for (int i = 0; i < indent; i++)
        fprintf(out, "    ");

    switch (n->type) {

    case AST_ASSIGNMENT:
        fprintf(out, "%s = ", n->left->value);
        emit_expr(n->right);
        fprintf(out, ";\n");
        break;

        case AST_FOR_STMT:
        fprintf(out, "for (");

        // init
        emit_for_part(n->left);
        fprintf(out, "; ");

        // condition
        emit_expr(n->right->body[0]);
        fprintf(out, "; ");

        // update
        emit_expr(n->right->body[1]);

        fprintf(out, ") ");
        emit_stmt(n->right->body[2], indent);
        break;


    case AST_FUNC_CALL:
        if (strcmp(n->value, "console.log") == 0) {
            SemType t = expr_type(n->body[0]);
            if (t == TYPE_STRING)
                fprintf(out, "printf(\"%%s\\n\", ");
            else
                fprintf(out, "printf(\"%%d\\n\", ");

            emit_expr(n->body[0]);
            fprintf(out, ");\n");
        }
        break;
    case AST_BLOCK:
        fprintf(out, "{\n");
        for (int i = 0; i < n->body_size; i++)
            emit_stmt(n->body[i], indent + 1);
        for (int i = 0; i < indent; i++)
            fprintf(out, "    ");
        fprintf(out, "}\n");
        break;

    case AST_IF_STMT:
        fprintf(out, "if (");
        emit_expr(n->left);
        fprintf(out, ") ");
        emit_stmt(n->right, indent);
        break;

    case AST_WHILE_STMT:
        fprintf(out, "while (");
        emit_expr(n->left);
        fprintf(out, ") ");
        emit_stmt(n->right, indent);
        break;

    default:
        break;
    }
}

void codegen_c(ASTNode *root, const char *out_file) {
    out = fopen(out_file, "w");
    if (!out) {
        perror("fopen");
        exit(1);
    }

    fprintf(out,
        "#include <stdio.h>\n\n"
        "#include <stdbool.h>\n\n"
        "int main() {\n");
    emit_declarations(root);
    if (root->type == AST_BLOCK) {
        for (int i = 0; i < root->body_size; i++)
            emit_stmt(root->body[i], 1);
    } else {
        emit_stmt(root, 1);
    }

    fprintf(out, "    return 0;\n}\n");
    fclose(out);
}
