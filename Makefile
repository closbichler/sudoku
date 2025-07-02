sudoku: sudoku.c sudoku.h exact_cover.h
	cc -o sudoku sudoku.c -Wall

clean: 
	rm sudoku