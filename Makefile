CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = \
src/main.c \
src/lexer/lexer.c \
src/parser/parser.c

OUT = jscc

all: $(OUT)

$(OUT):
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT) tokens.txt
