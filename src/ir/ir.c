#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/ir.h"

static int tempCount = 0;
static int labelCount = 0;
static IRInstr ir[1024];
static int ir_count = 0;

static void emit(IRInstr i)
{
    ir[ir_count++] = i;
}

static char *strdup_safe(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy)
    {
        perror("malloc");
        exit(1);
    }
    memcpy(copy, s, len);
    return copy;
}

IRInstr *ir_get_all(int *count) {
    *count = ir_count;
    return ir;
}


static char *new_temp()
{
    static char buf[16];
    snprintf(buf, sizeof(buf), "t%d", tempCount++);
    return buf;
}

static char *new_label()
{
    static char buf[16];
    snprintf(buf, sizeof(buf), "L%d", labelCount++);
    return buf;
}

static char *gen_expr(ASTNode *node)
{
    if (!node)
        return "";

    switch (node->type)
    {
    case AST_LITERAL:
    case AST_IDENTIFIER:
        return node->value;

    case AST_BINARY_OP:
    {
        char *l = gen_expr(node->left);
        char *r = gen_expr(node->right);
        char *t = strdup_safe(new_temp());
        emit((IRInstr){
            .op = IR_BINOP,
            .dst = t,
            .lhs = l,
            .op_str = node->value,
            .rhs = r});
        return t;
    }

    default:
        return "";
    }
}

static void gen_stmt(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {

    case AST_ASSIGNMENT:
    {
        char *rhs = gen_expr(node->right);
        emit((IRInstr){
            .op = IR_ASSIGN,
            .dst = node->left->value,
            .lhs = rhs});

        break;
    }

    case AST_IF_STMT:
    {
        char *cond = gen_expr(node->left);
        char *Lfalse = new_label();
        emit((IRInstr){
            .op = IR_IF_FALSE,
            .lhs = cond,
            .label = Lfalse});
        gen_stmt(node->right);
        emit((IRInstr){
            .op = IR_LABEL,
            .label = Lfalse});
        break;
    }

    case AST_WHILE_STMT:
    {
        char *Lstart = new_label();
        char *Lend = new_label();
        emit((IRInstr){
            .op = IR_LABEL,
            .label = Lstart});
        char *cond = gen_expr(node->left);
        emit((IRInstr){
            .op = IR_IF_FALSE,
            .lhs = cond,
            .label = Lend});
        gen_stmt(node->right);
        emit((IRInstr){
            .op = IR_GOTO,
            .label = Lstart});
        emit((IRInstr){
            .op = IR_LABEL,
            .label = Lend});
        break;
    }

    case AST_BLOCK:
        for (int i = 0; i < node->body_size; i++)
            gen_stmt(node->body[i]);
        break;

    case AST_FUNC_CALL:
        for (int i = 0; i < node->body_size; i++)
        {
            char *arg = gen_expr(node->body[i]);
            emit((IRInstr){
                .op = IR_PARAM,
                .lhs = arg});
        }
        emit((IRInstr){
            .op = IR_CALL,
            .func = node->value,
            .argc = node->body_size});
        break;

    default:
        break;
    }
}

void ir_generate(ASTNode *root)
{
    gen_stmt(root);
}
