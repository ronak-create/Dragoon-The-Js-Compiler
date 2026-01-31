#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/ir.h"
#include "../include/cfg.h"
#include "../include/opt.h"
#include "../include/codegen.h"
#include "../include/qbe_codegen.h"
#include <sys/stat.h>
#include <sys/types.h>

void ensure_tmp_dir(void) {
#ifdef _WIN32
    _mkdir("tmp");
#else
    mkdir("tmp", 0755);
#endif
}

int main(int argc, char *argv[])
{
    ensure_tmp_dir();

    int debug = 0;
    int stop_at_qbe = 0;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-d"))
            debug = 1;
        if (!strcmp(argv[i], "-q"))
            stop_at_qbe = 1;
    }
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        printf("Error opening file: %s\n", argv[1]);
        return 1;
    }

    FILE *outputFile = fopen("tokens.txt", "w");
    if (!outputFile)
    {
        printf("Error opening output file.\n");
        fclose(file);
        return 1;
    }


    if (debug)
    {
        printf("Token: ...\n");
    }

    Token tokens[1000];
    int tokenCount = 0;
    Token token;

    do
    {
        token = get_next_token(file);
        tokens[tokenCount++] = token;

        const char *tokenType =
            token.type == TOKEN_IDENTIFIER ? "TOKEN_IDENTIFIER" : token.type == TOKEN_KEYWORD   ? "TOKEN_KEYWORD"
                                                              : token.type == TOKEN_NUMBER      ? "TOKEN_NUMBER"
                                                              : token.type == TOKEN_STRING      ? "TOKEN_STRING"
                                                              : token.type == TOKEN_OPERATOR    ? "TOKEN_OPERATOR"
                                                              : token.type == TOKEN_PARENTHESES ? "TOKEN_PARENTHESES"
                                                              : token.type == TOKEN_SEMICOLON   ? "TOKEN_SEMICOLON"
                                                              : token.type == TOKEN_PUNCTUATION ? "TOKEN_PUNCTUATION"
                                                              : token.type == TOKEN_COMMENT     ? "TOKEN_COMMENT"
                                                              : token.type == TOKEN_BOOLEAN     ? "TOKEN_BOOLEAN"
                                                              : token.type == TOKEN_ERROR       ? "TOKEN_ERROR"
                                                                                                : "TOKEN_EOF";
        if (debug)
        {            
            printf("Token: %-17s | Lexeme: %-15s | Line: %d\n", tokenType, token.lexeme, token.line);
        }
            
        if (token.type != TOKEN_EOF)
        {
            fprintf(outputFile, "    {%s, \"%s\", %d},\n", tokenType, token.lexeme, token.line);
        }
    } while (token.type != TOKEN_EOF);
    fprintf(outputFile, "};\n");
    fclose(outputFile);
    fclose(file);

    // Now parse and generate IR from tokens[]
    // Parse entire program
    int index = 0;

    ASTNode *program = create_node(AST_BLOCK, NULL);
    program->body = malloc(sizeof(ASTNode *) * 256);
    program->body_size = 0;

    while (tokens[index].type != TOKEN_ERROR &&
           tokens[index].type != TOKEN_EOF )
    {

        ASTNode *stmt = parse_statement(tokens, &index);
        if (stmt)
        {
            program->body[program->body_size++] = stmt;
        }
    }

    // Semantic analysis (ONE PASS)
    semantic_analyze(program);

    // Constant Folding
    program = opt_fold_constants(program);
    if (debug)
    {
        printf("\n=== After Constant Folding ===\n");
        // printf(program);
        print_ast(program, 0);
    }
    // IR/TAC Generation
    if (debug)
    {
        printf("\n===IR / TAC ===\n");
    }
    
    ir_generate(program);

    // Control Flow Graph Construction
    if(debug)
    {
        printf("\n=== CFG ===\n");
    }
    cfg_build(program);
    if(debug)
        cfg_print();

    // Dead Code Elimination
    opt_dead_code_elimination();

    //     /* =========================
    //    QBE Backend
    //    ========================= */
    int ir_count;
    IRInstr *ir = ir_get_all(&ir_count);

    qbe_codegen_ir(ir, ir_count, "./tmp/out.qbe");

    if (stop_at_qbe)
    {
        printf("Generated QBE IR → tmp/out.qbe\n");
        return 0;
    }
    system("./qbe tmp/out.qbe > tmp/out.s");
    system("gcc tmp/out.s -o out");

    printf("Running program:\n");
    system("./out");

    // printf("\nGenerated TAC:\n");
    // for (int i = 0; i < astCount; i++) {
    //     generate_tac(astList[i]);
    // }

    // printf("\nSymbol Table:\n");
    // printSymbolTable();

    // for (int i = 0; i < astCount; i++) {
    //     free_ast(astList[i]);
    // }

    return 0;
}