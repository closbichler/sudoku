all: test web

test: sudoku.h sudoku_test.c sudoku_solver.h examples.h
	cc -o sudoku_test sudoku_test.c -Wall

web: sudoku.h sudoku_solver.h examples.h
	clang --target=wasm32 -nostdlib -c sudoku.c
	wasm-ld --no-entry --export-all --allow-undefined \
		-o ./public/sudoku.wasm sudoku.o

clean: 
	rm sudoku_test