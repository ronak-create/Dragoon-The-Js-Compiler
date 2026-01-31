CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -g -Iinclude
LDFLAGS =

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
TMP = tmp
FILE ?= tests/index.js

.PHONY: all clean run qbe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

# Run compiler on a JS file (default: tests/index.js)
run: $(OUT)
	mkdir -p $(TMP)
	./$(OUT) $(FILE)

# Stop at QBE stage
qbe: $(OUT)
	mkdir -p $(TMP)
	./$(OUT) $(FILE) -q

clean:
	rm -f $(OUT)
	rm -rf $(TMP)
	rm -f out
