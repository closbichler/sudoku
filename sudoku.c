#include <stdio.h>
#include <time.h>

#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"

int pseudorandom(int n) 
{
    return rand() % n;
}

void measure_solver(Sudoku s) 
{
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int solutions = sudoku_get_solutions(s);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
}

int main() 
{
    srand(time(NULL));

    Sudoku s = {0}, solved = {0};
    int attempts = 0;
    while (attempts < 10) {
        attempts++;
        printf("attempt %d\n", attempts);
        s = sudoku_create(9, 3, pseudorandom, 17);
        solved = (Sudoku) {
            .size = s.size,
            .block_size = s.block_size
        };
        memcpy(&solved.field, &s.field, sizeof(s.field));
        printf("checking if solvable..\n");
        if (sudoku_is_solvable(solved)) break;
    }
    sudoku_print(s);
    sudoku_print(solved);

    /*
    easy_sudoku
    printf("--easy--\n");
    measure_solver(sudoku);
    
    medium_sudoku
    printf("--medium--\n");
    measure_solver(sudoku);
 
    hard_sudoku
    printf("--hard--\n");
    measure_solver(sudoku);
    
    very_hard_sudoku
    printf("--very hard--\n");
    measure_solver(sudoku);

    non_proper_sudoku
    printf("--non proper--\n");
    measure_solver(sudoku);
    */
 
    return 0;
}