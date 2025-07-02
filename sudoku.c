#include <stdio.h>
#include <time.h>

#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"
#include "examples.h"

#define EXACT_COVER_IMPLEMENTATION
#include "exact_cover.h"

int pseudorandom(int n) 
{
    return rand() % n;
}

void measure_solvers(Sudoku s) 
{
    clock_t start, end;
    double cpu_time_used;

    printf("Exact-cover: ");
    start = clock();
    int solutions = exact_cover_sudoku_solutions(s);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
    
    printf("Brute-force: ");
    start = clock();
    solutions = sudoku_get_solutions(s);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
}

int main() 
{
    srand(time(NULL));

/*
    Sudoku s = {0}, solved = {0};
    int attempts = 0;
    while (attempts < 10) {
        attempts++;
        printf("attempt %d\n", attempts);
        s = sudoku_create(9, 3, pseudorandom, 17);
        solved = sudoku_create_empty(s.size, s.block_size);
        for (int i=0; i<s.size; i++) {
            for (int j=0; j<s.size; j++) {
                solved.field[i][j] = s.field[i][j];
            }
        }
        printf("checking if solvable..\n");
        if (exact_cover_solve_sudoku(&solved)) break;
    }
    sudoku_print(s);
    sudoku_print(solved);
*/

    Sudoku s = {0};
    printf("--easy--\n");
    sudoku_example_easy(&s);
    measure_solvers(s);
    
    printf("--medium--\n");
    sudoku_example_medium(&s);
    measure_solvers(s);
    
    printf("--hard--\n");
    sudoku_example_hard(&s);
    measure_solvers(s);

    /*printf("--very hard--\n");
    sudoku_example_very_hard(&s);
    measure_solvers(s);
    sudoku_print(s);
    */
    printf("--non proper--\n");
    sudoku_example_non_proper(&s);
    measure_solvers(s);
    
    return 0;
}