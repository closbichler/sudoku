all: test web

test: sudoku.h sudoku_test.c sus.h examples.h
	cc -o sudoku_test sudoku_test.c -Wall

web: sudoku.h sus.h examples.h
	clang --target=wasm32 -nostdlib -c sudoku.c
	wasm-ld --no-entry --export-all --allow-undefined \
		-o ./public/sudoku.wasm sudoku.o

clean: 
	rm sudoku_test