#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"

#define SUS_IMPLEMENTATION
#include "sus.h"

#include "examples.h"

// -- Test and debug helper functions -- 

void sudoku_print(Sudoku s)
{
    if (s.field == NULL) { fprintf(stdout, "empty sudoku\n"); return; }
    if (sudoku_is_valid(s)) fprintf(stdout, "[valid] ");
    else                    fprintf(stdout, "[invalid] ");
    fprintf(stdout, "Sudoku %dx%d with block-size %d\n", s.size, s.size, s.block_size);

    for (int i=0; i<s.size; i++) {
        if (i % s.block_size == 0) {
            fprintf(stdout, "|");
            for (int k=0; k<s.size + s.block_size/2; k++) fprintf(stdout, "--");
            fprintf(stdout, "|\n");
        }
        for (int j=0; j<s.size; j++) {
            if (j % s.block_size == 0) fprintf(stdout, "|");
            fprintf(stdout, "|");
            if (s.field[i][j] == 0)
                fprintf(stdout, " ");
            else
                fprintf(stdout, "%d", s.field[i][j]);
        }
        fprintf(stdout, "|\n");
    }
    fprintf(stdout, "|");
    for (int k=0; k<s.size+s.block_size/2; k++) fprintf(stdout, "--");
    fprintf(stdout, "|\n");
}

// -- Solve real sudokus --

void solve_and_print_sudoku(Sudoku s) 
{
    sus_solve_sudoku(&s);
    sudoku_print(s);   
    printf("\n");
}

int pseudorandom(int n) 
{
    return rand() % n;
}

// -- Performance Tests --

double measure_and_assert_solver(ulong (*F)(Sudoku), Sudoku s, ulong num_solutions) 
{
    clock_t start, end;
    double cpu_time_used;
    ulong solutions;

    start = clock();
    solutions = F(s);
    end = clock();
    if (num_solutions != -1 && solutions != num_solutions) {
        fprintf(stderr, "\x1b[31m Expected %ld solutions but got %ld.\x1b[0m\n", num_solutions, solutions);
    }
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    return cpu_time_used;
}

void print_performance_row(const char *name, double a, double b)
{
    const char *color = "\x1b[36m"; // cyan
    const char *reset = "\x1b[0m";
    char s0[64], s1[64];

    if (a == 0) snprintf(s0, sizeof s0, "N/A");
    else        snprintf(s0, sizeof s0, "%.4fs", a);
    if (b == 0) snprintf(s1, sizeof s1, "N/A");
    else        snprintf(s1, sizeof s1, "%.4fs", b);

    int pos1 = 35, pos2 = 56;
    int cur = 0;

    fprintf(stdout, "%s ", name);
    cur = (int)strlen(name) + 1;

    while (cur < pos1) { fputc('.', stdout); cur++; }
    fprintf(stdout, " %s%s%s", color, s0, reset); cur += 1 + (int)strlen(s0);

    while (cur < pos2) { fputc('.', stdout); cur++; }
    fprintf(stdout, " %s%s%s", color, s1, reset); cur += 1 + (int)strlen(s1);
    fprintf(stdout, "\n");
}

// -- Unit Tests -- 

bool test_solve_sudoku_1() 
{
    Sudoku s = {0};
    sudoku_example_easy(&s);
    if(!sus_solve_sudoku(&s)) return false;
    if(!sudoku_is_valid(s)) return false;
    return true;
}

bool test_solve_sudoku_2() 
{
    Sudoku s = {0};
    sudoku_example_medium(&s);
    if(!sus_solve_sudoku(&s)) return false;
    if(!sudoku_is_valid(s)) return false;
    return true;
}

bool test_count_solutions()
{
    Sudoku s1 = {0};
    sudoku_example_easy(&s1);
    if(sus_count_solutions(s1) != 1) return false;

    Sudoku s2 = {0};
    sudoku_example_multiple_solutions(&s2);
    ulong result = sus_count_solutions(s2);
    if(result != 2761) return false;
    
    Sudoku s3 = {0};
    sudoku_example_no_solutions(&s3);
    if(sus_count_solutions(s3) != 0) return false;
    return true;
}

int main(int argc, char *argv[])
{
    bool test_unit        = true;
    bool test_performance = true;
    bool test_real        = false;
    bool test_generate    = false;

    fprintf(stdout, "\n\x1b[36m\x1b[1m========================================\n");
    fprintf(stdout, "  \x1b[33mSUS — Unit Tests\x1b[36m\n");
    fprintf(stdout, "========================================\x1b[0m\n\n");

    fprintf(stdout, "Unit test summary:\n");
    if (!test_unit) {
        fprintf(stdout, "skipped.\n\n");
    } else {
        int num_tests = 3;
        char* test_names[num_tests];
        bool test_results[num_tests];
        
        test_names[0] = "solve_sudoku_1";
        test_results[0] = test_solve_sudoku_1();
        test_names[1] = "solve_sudoku_2";
        test_results[1] = test_solve_sudoku_2();
        test_names[2] = "count_solutions";
        test_results[2] = test_count_solutions();
        
        fprintf(stdout, "\nTest name                                             Result\n");
        for (int i = 0; i < num_tests; i++) {
            const char *result = test_results[i] ? "PASSED" : "FAILED";
            const char *color = test_results[i] ? "\x1b[32m" : "\x1b[31m"; // green / red
            const char *reset = "\x1b[0m";
            int total_width = 60;
            int name_len = (int)strlen(test_names[i]);
            int result_len = (int)strlen(result);
            int dots = total_width - name_len - result_len - 2; // spaces around dots
            if (dots < 1) dots = 1;

            fprintf(stdout, "%s ", test_names[i]);
            for (int d = 0; d < dots; d++) fputc('.', stdout);
            fprintf(stdout, " %s%s%s\n", color, result, reset);
        }
        fprintf(stdout, "\n\n");
    }

    
    fprintf(stdout, "Performance test summary: \n");
    if (!test_performance) {
        fprintf(stdout, "skipped.\n\n");
    } else {
        int num_tests = 7;
        char *performance_test_names[num_tests];
        double performance_test_results[num_tests][2];
        Sudoku s = {0};

        sudoku_example_wrong(&s);        
        performance_test_names[0] = "wrong";
        performance_test_results[0][0] = measure_and_assert_solver(sus_count_solutions, s, 0);
        performance_test_results[0][1] = measure_and_assert_solver(sudoku_get_solutions, s, 0);

        sudoku_example_easy(&s);
        performance_test_names[1] = "easy";
        performance_test_results[1][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[1][1] = measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_medium(&s);
        performance_test_names[2] = "medium";
        performance_test_results[2][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[2][1] = measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_hard(&s);
        performance_test_names[3] = "hard";
        performance_test_results[3][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[3][1] = measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_very_hard(&s);
        performance_test_names[4] = "very hard";
        performance_test_results[4][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[4][1] = 0;// measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_multiple_solutions(&s);
        performance_test_names[5] = "multiple solutions";
        performance_test_results[5][0] = measure_and_assert_solver(sus_count_solutions, s, 2761);
        performance_test_results[5][1] = 0; // measure_and_assert_solver(sudoku_get_solutions, s, 2761);

        sudoku_example_even_more_solutions(&s);
        performance_test_names[6] = "even more solutions";
        performance_test_results[6][0] = measure_and_assert_solver(sus_count_solutions, s, -1);
        performance_test_results[6][1] = 0; // measure_and_assert_solver(sudoku_get_solutions, s, 2761);

        fprintf(stdout, "\nTest name                           SUS                  Brute Force\n");
        for (int i = 0; i < num_tests; i++) {
            print_performance_row(performance_test_names[i],
                performance_test_results[i][0],
                performance_test_results[i][1]);
        }
        fprintf(stdout, "\n\n");
    }
  
    fprintf(stdout, "Real sudoku tests: \n");
    if (!test_real) {
        fprintf(stdout, "skipped.\n\n");
    } else {
        fprintf(stdout, "\n");

        Sudoku s = {0};

        sudoku_example_hard(&s);
        solve_and_print_sudoku(s);
        
        sudoku_example_4x4(&s);
        solve_and_print_sudoku(s);

        sudoku_example_16x16(&s);
        solve_and_print_sudoku(s);
        
        fprintf(stdout, "\n\n");
    }

    fprintf(stdout, "Generate sudokus: \n");
    if (!test_generate) {
        fprintf(stdout, "skipped.\n\n");
    } else {
        fprintf(stdout, "\n");

        srand(time(0));

        Sudoku s_4x4 = sus_generate_sudoku(4, 2, 5);
        sudoku_print(s_4x4);
        fprintf(stdout, "Solutions: %ld\n", sus_count_solutions(s_4x4));
        solve_and_print_sudoku(s_4x4);

        Sudoku s_9x9 = sus_generate_sudoku(9, 3, 26);
        sudoku_print(s_9x9);
        fprintf(stdout, "Solutions: %ld\n", sus_count_solutions(s_9x9));
        solve_and_print_sudoku(s_9x9);

        // Sudoku s_16x16 = sus_generate_sudoku(16, 4, 16*16);
        // sudoku_print(s_16x16);
        // fprintf(stdout, "Solutions: %ld\n", sus_count_solutions(s_16x16));
        // solve_and_print_sudoku(s_16x16);
    }

    return 0;
}
