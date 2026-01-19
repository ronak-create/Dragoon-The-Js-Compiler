#include <stdio.h>
#include <stdlib.h>
#include "../include/cfg.h"

#define MAX_BLOCKS 128

static BasicBlock *blocks[MAX_BLOCKS];
static int block_count = 0;

static BasicBlock *new_block() {
    BasicBlock *b = malloc(sizeof(BasicBlock));
    b->id = block_count;
    b->succ = NULL;
    b->succ_count = 0;
    b->stmts = NULL;
    b->stmt_count = 0;
    blocks[block_count++] = b;
    return b;
}

static void add_edge(BasicBlock *from, BasicBlock *to) {
    from->succ = realloc(from->succ,
        sizeof(BasicBlock*) * (from->succ_count + 1));
    from->succ[from->succ_count++] = to;
}

static BasicBlock *build_from_node(ASTNode *node, BasicBlock *curr) {
    if (!node) return curr;

    switch (node->type) {

    case AST_BLOCK:
        for (int i = 0; i < node->body_size; i++)
            curr = build_from_node(node->body[i], curr);
        return curr;

    case AST_IF_STMT: {
        BasicBlock *thenB = new_block();
        BasicBlock *after = new_block();

        add_edge(curr, thenB);
        add_edge(curr, after);

        BasicBlock *endThen = build_from_node(node->right, thenB);
        add_edge(endThen, after);

        return after;
    }

    case AST_WHILE_STMT: {
        BasicBlock *cond = new_block();
        BasicBlock *body = new_block();
        BasicBlock *after = new_block();

        add_edge(curr, cond);
        add_edge(cond, body);
        add_edge(cond, after);

        BasicBlock *endBody = build_from_node(node->right, body);
        add_edge(endBody, cond);

        return after;
    }

    default:
        curr->stmts = realloc(curr->stmts,
            sizeof(ASTNode*) * (curr->stmt_count + 1));
        curr->stmts[curr->stmt_count++] = node;
        return curr;
    }
}

void cfg_build(ASTNode *root) {
    block_count = 0;
    BasicBlock *entry = new_block();
    build_from_node(root, entry);
}

void cfg_print() {
    for (int i = 0; i < block_count; i++) {
        BasicBlock *b = blocks[i];
        printf("Block B%d:\n", b->id);
        printf("  Statements: %d\n", b->stmt_count);
        printf("  Successors:");
        for (int j = 0; j < b->succ_count; j++)
            printf(" B%d", b->succ[j]->id);
        printf("\n\n");
    }
}

BasicBlock *cfg_get_block(int index) {
    if (index < 0 || index >= block_count) return NULL;
    return blocks[index];
}

int cfg_block_count(void) {
    return block_count;
}
