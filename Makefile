CC = gcc
CFLAGS = -Wall -Wextra

all: test web exact_cover_cli sudoku_cli

test: sudoku.h sudoku_test.c sus.h exact_cover.h
	$(CC) -o sudoku_test sudoku_test.c $(CFLAGS)
	$(CC) -o exact_cover_test exact_cover_test.c $(CFLAGS)

web: sudoku.h sus.h sus_glue.c extern/walloc.c
	clang --target=wasm32 -nostdlib -c -o walloc.o extern/walloc.c
	clang --target=wasm32 -nostdlib -c -o sus_glue.o sus_glue.c
	wasm-ld --no-entry -o ./public/sudoku.wasm sus_glue.o walloc.o

exact_cover_cli: exact_cover.h exact_cover_cli.c
	$(CC) -o exact_cover_cli exact_cover_cli.c $(CFLAGS)

sudoku_cli: sudoku.h sudoku_cli.c
	$(CC) -o sudoku_cli sudoku_cli.c $(CFLAGS)

clean: 
	rm -f sudoku_test sus_glue.o walloc.o public/sudoku.wasm sudoku_test.su exact_cover_test exact_cover_cli