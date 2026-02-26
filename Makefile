all: sudoku-test sudoku-web

sudoku-test: sudoku_test.c sudoku.h sudoku_solver.h examples.h
	cc -o sudoku_test sudoku_test.c -Wall

sudoku-web: 
	clang --target=wasm32 -nostdlib -c sudoku.c
	wasm-ld --no-entry --export-all --allow-undefined \
		-o ./public/sudoku.wasm sudoku.o

clean: 
	rm sudoku_test