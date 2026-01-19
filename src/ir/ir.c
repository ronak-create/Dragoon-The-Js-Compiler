#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/ir.h"

static int tempCount = 0;
static int labelCount = 0;

static char *strdup_safe(const char *s)
{
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy) {
        perror("malloc");
        exit(1);
    }
    memcpy(copy, s, len);
    return copy;
}

static char *new_temp() {
    static char buf[16];
    snprintf(buf, sizeof(buf), "t%d", tempCount++);
    return buf;
}

static char *new_label() {
    static char buf[16];
    snprintf(buf, sizeof(buf), "L%d", labelCount++);
    return buf;
}

static char *gen_expr(ASTNode *node) {
    if (!node) return "";

    switch (node->type) {
    case AST_LITERAL:
    case AST_IDENTIFIER:
        return node->value;

    case AST_BINARY_OP: {
        char *l = gen_expr(node->left);
        char *r = gen_expr(node->right);
        char *t = strdup_safe(new_temp());
        printf("%s = %s %s %s\n", t, l, node->value, r);
        return t;
    }

    default:
        return "";
    }
}

static void gen_stmt(ASTNode *node) {
    if (!node) return;

    switch (node->type) {

    case AST_ASSIGNMENT: {
        char *rhs = gen_expr(node->right);
        printf("%s = %s\n", node->left->value, rhs);
        break;
    }

    case AST_IF_STMT: {
        char *cond = gen_expr(node->left);
        char *Lfalse = new_label();
        printf("ifFalse %s goto %s\n", cond, Lfalse);
        gen_stmt(node->right);
        printf("%s:\n", Lfalse);
        break;
    }

    case AST_WHILE_STMT: {
        char *Lstart = new_label();
        char *Lend = new_label();
        printf("%s:\n", Lstart);
        char *cond = gen_expr(node->left);
        printf("ifFalse %s goto %s\n", cond, Lend);
        gen_stmt(node->right);
        printf("goto %s\n", Lstart);
        printf("%s:\n", Lend);
        break;
    }

    case AST_BLOCK:
        for (int i = 0; i < node->body_size; i++)
            gen_stmt(node->body[i]);
        break;

    case AST_FUNC_CALL:
        for (int i = 0; i < node->body_size; i++) {
            char *arg = gen_expr(node->body[i]);
            printf("param %s\n", arg);
        }
        printf("call %s, %d\n", node->value, node->body_size);
        break;

    default:
        break;
    }
}

void ir_generate(ASTNode *root) {
    gen_stmt(root);
}
