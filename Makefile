all: test web

test: sudoku.h sudoku_test.c sus.h examples.h
	gcc -o sudoku_test sudoku_test.c -Wall

web: sudoku.h sus.h examples.h
	clang --target=wasm32 -nostdlib -c -o walloc.o lib/walloc.c
	clang --target=wasm32 -nostdlib -c -o sus_glue.o sus_glue.c
	wasm-ld --no-entry -o ./public/sudoku.wasm sus_glue.o walloc.o

clean: 
	rm -f sudoku_test sus_glue.o walloc.o public/sudoku.wasm sudoku_test.su