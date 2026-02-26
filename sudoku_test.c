#include <stdio.h>
#include <time.h>

#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"
#include "examples.h"

#define SUDOKU_SOLVER_IMPLEMENTATION
#include "sudoku_solver.h"

int pseudorandom(int n) 
{
    return rand() % n;
}

void measure_and_assert_solvers(const char* name, Sudoku s, int num_solutions) 
{
    clock_t start, end;
    double cpu_time_used;
    int solutions;

    printf("--%s--\n", name);

    printf("Exact-cover:       ");
    start = clock();
    solutions = solver_exact_cover_count_solutions(s);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
    assert(solutions == num_solutions);
    
    // printf("Exact-cover (dlx): ");
    // start = clock();
    // solutions = solver_count_solutions(s);
    // end = clock();
    // cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    // printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
    // assert(solutions == num_solutions);
    
    // printf("Brute-force:       ");
    // start = clock();
    // solutions = sudoku_get_solutions(s);
    // end = clock();
    // cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    // printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
    // assert(solutions == num_solutions);

    printf("\n");
}

void test_solvers()
{
    Sudoku s = {0};
    sudoku_example_wrong(&s);
    measure_and_assert_solvers("wrong", s, 0);

    sudoku_example_easy(&s);
    measure_and_assert_solvers("easy", s, 1);
    
    sudoku_example_medium(&s);
    measure_and_assert_solvers("medium", s, 1);
    
    sudoku_example_hard(&s);
    measure_and_assert_solvers("hard", s, 1);

    sudoku_example_very_hard(&s);
    measure_and_assert_solvers("very hard", s, 1);
    
    sudoku_example_multiple_solutions(&s);
    measure_and_assert_solvers("multiple solutions", s, 2761);

    sudoku_example_no_solutions(&s);
    measure_and_assert_solvers("no solutions (unsolvable)", s, 0);
}

void generate_and_solve_random_sudoku() 
{
    srand(time(0));

    Sudoku s = {0};
    int attempts = 0;
    while (attempts < 20) {
        attempts++;
        printf("attempt %d\n", attempts);
        s = sudoku_create(9, 3, pseudorandom, 14);
        printf("checking if solvable..\n");
        int solutions = solver_exact_cover_count_solutions(s);
        if (solutions == 1) break;
        else printf("found %d solutions, trying again..\n", solutions);
    }
    solver_exact_cover_fill_sudoku(&s);
    sudoku_print(s);   
}

int main() 
{
    // test_solvers();
    generate_and_solve_random_sudoku();

    return 0;
}