#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opt.h"

static int is_number(const char *s) {
    for (int i = 0; s[i]; i++)
        if (s[i] < '0' || s[i] > '9')
            return 0;
    return 1;
}

static ASTNode *fold_node(ASTNode *n) {
    if (!n) return NULL;

    n->left = fold_node(n->left);
    n->right = fold_node(n->right);

    if (n->type == AST_BINARY_OP &&
        n->left && n->right &&
        n->left->type == AST_LITERAL &&
        n->right->type == AST_LITERAL &&
        is_number(n->left->value) &&
        is_number(n->right->value)) {

        int a = atoi(n->left->value);
        int b = atoi(n->right->value);
        int res = 0;

        if (!strcmp(n->value, "+")) res = a + b;
        else if (!strcmp(n->value, "-")) res = a - b;
        else if (!strcmp(n->value, "*")) res = a * b;
        else if (!strcmp(n->value, "/")) res = b ? a / b : 0;
        else return n;

        char buf[32];
        snprintf(buf, sizeof(buf), "%d", res);
        return create_node(AST_LITERAL, buf);
    }

    return n;
}

void opt_constant_folding(void) {
    // For now, folding happens before IR, so this is a hook
    // Call fold_node(program_ast) in main
}

static void dfs(BasicBlock *b, int *visited) {
    if (!b || visited[b->id]) return;
    visited[b->id] = 1;

    for (int i = 0; i < b->succ_count; i++)
        dfs(b->succ[i], visited);
}

void opt_dead_code_elimination(void) {
    int count = cfg_block_count();
    int visited[128] = {0};

    dfs(cfg_get_block(0), visited);

    for (int i = 0; i < count; i++) {
        if (!visited[i]) {
            BasicBlock *b = cfg_get_block(i);
            printf("DCE: removing unreachable block B%d\n", b->id);
            b->stmt_count = 0;
        }
    }
}

ASTNode *opt_fold_constants(ASTNode *root) {
    return fold_node(root);
}
