sudoku: sudoku.c sudoku.h sudoku_solver.h examples.h
	cc -o sudoku sudoku.c -Wall

clean: 
	rm sudoku