CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = \
src/main.c \
src/lexer/lexer.c \
src/parser/parser.c \
src/semantic/semantic.c \
src/ir/ir.c \
src/cfg/cfg.c \
src/opt/opt.c \
src/codegen/codegen.c \
src/qbe/qbe_codegen.c

OUT = jscc

all: $(OUT)

$(OUT):
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f jscc out.c out.qbe out.s
