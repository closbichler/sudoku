#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#define SUDOKU_IMPLEMENTATION
// #define EXACT_COVER_DEBUG
#include "sudoku.h"

#define SUS_IMPLEMENTATION
#include "sus.h"

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

ulong count_solutions_with_dp_wrapper(Sudoku s) {
    return sus_count_solutions_until(s, 10000, 1);
}

bool parse_sudoku(FILE *file, Sudoku *s) 
{
    char line[4096];
    fgets(line, sizeof(line), file);

    if (strncmp(line, "sudoku ", 6) != 0) {
        fprintf(stderr, "\x1b[31m Invalid file format: expected line starting with 'sudoku '.\x1b[0m\n");
        return false;
    }

    int size, block_size;

    char *endptr;
    errno = 0;
    long size_long = strtol(line + 6, &endptr, 10);
    if (endptr == line + 6 || errno == ERANGE || size_long < 0 || size_long > INT_MAX) {
        fprintf(stderr, "\x1b[31m Invalid sudoku size: %ld.\x1b[0m\n", size_long);
        return false;
    }

    char *block_start = endptr + 1;
    errno = 0;
    long block_size_long = strtol(block_start, &endptr, 10);
    if (endptr == block_start || errno == ERANGE || block_size_long < 0 || block_size_long > INT_MAX) {
        fprintf(stderr, "\x1b[31m Invalid block size: %ld.\x1b[0m\n", block_size_long);
        return false;
    }

    size = (int)size_long;
    block_size = (int)block_size_long;

    *s = sudoku_create_empty(size, block_size);
    for (int i = 0; i < size; i++) {
        fgets(line, sizeof(line), file);
        char *p = line;
        for (int j = 0; j < size; j++) {
            int val = (int)strtol(p, &p, 10);
            s->field[i][j] = (uint8_t)val;
        }
    }
    
    return true;
}

bool read_sudoku_from_file(const char *filename, Sudoku *s) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "\x1b[31m Failed to open file %s.\x1b[0m\n", filename);
        return false;
    }
    bool result = parse_sudoku(file, s);
    fclose(file);
    return result;
}

double measure_and_assert_solver(ulong (*F)(Sudoku), const char *filename, ulong num_solutions) 
{
    Sudoku s = {0};
    if (!read_sudoku_from_file(filename, &s)) {
        fprintf(stderr, "\x1b[31m Failed to read sudoku from file %s.\x1b[0m\n", filename);
        return 0;
    }

    clock_t start, end;
    double cpu_time_used;
    ulong solutions;

    start = clock();
    solutions = F(s);
    end = clock();
    if (num_solutions != -1UL && solutions != num_solutions) {
        fprintf(stderr, "\x1b[31m Expected %ld solutions but got %ld.\x1b[0m\n", num_solutions, solutions);
    }
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    return cpu_time_used;
}

void print_performance_row(const char *name, double a, double b, double c)
{
    const char *color = "\x1b[36m"; // cyan
    const char *reset = "\x1b[0m";
    char s0[64], s1[64];
    char s2[64];

    if (a == 0) snprintf(s0, sizeof s0, "N/A");
    else        snprintf(s0, sizeof s0, "%.4fs", a);
    if (b == 0) snprintf(s1, sizeof s1, "N/A");
    else        snprintf(s1, sizeof s1, "%.4fs", b);
    if (c == 0) snprintf(s2, sizeof s2, "N/A");
    else        snprintf(s2, sizeof s2, "%.4fs", c);

    int pos1 = 35, pos2 = 56, pos3 = 79;
    int cur = 0;

    fprintf(stdout, "%s ", name);
    cur = (int)strlen(name) + 1;

    while (cur < pos1) { fputc('.', stdout); cur++; }
    fprintf(stdout, " %s%s%s", color, s0, reset); cur += 1 + (int)strlen(s0);

    while (cur < pos2) { fputc('.', stdout); cur++; }
    fprintf(stdout, " %s%s%s", color, s1, reset); cur += 1 + (int)strlen(s1);

    while (cur < pos3) { fputc('.', stdout); cur++; }
    fprintf(stdout, " %s%s%s", color, s2, reset); cur += 1 + (int)strlen(s2);
    fprintf(stdout, "\n");
}

// -- Unit Tests -- 

bool test_solve_sudoku_1() 
{
    Sudoku s = {0};
    if (!read_sudoku_from_file("examples/sudoku/9x9-easy.txt", &s)) return false;
    if(!sus_solve_sudoku(&s)) return false;
    if(!sudoku_is_valid(s)) return false;
    return true;
}

bool test_solve_sudoku_2() 
{
    Sudoku s = {0};
    if (!read_sudoku_from_file("examples/sudoku/9x9-medium.txt", &s)) return false;
    if(!sus_solve_sudoku(&s)) return false;
    if(!sudoku_is_valid(s)) return false;
    return true;
}

bool test_count_solutions()
{
    Sudoku s1 = {0};
    if (!read_sudoku_from_file("examples/sudoku/9x9-easy.txt", &s1)) return false;
    if(sus_count_solutions(s1) != 1) return false;

    Sudoku s2 = {0};
    if (!read_sudoku_from_file("examples/sudoku/9x9-multiple-solutions.txt", &s2)) return false;
    ulong result = sus_count_solutions(s2);
    if(result != 2761) return false;
    
    Sudoku s3 = {0};
    if (!read_sudoku_from_file("examples/sudoku/9x9-no-solutions.txt", &s3)) return false;
    if(sus_count_solutions(s3) != 0) return false;
    return true;
}

int main()
{
    bool test_unit        = true;
    bool test_performance = true;
    bool test_generate    = false;

    fprintf(stdout, "\n\x1b[36m\x1b[1m========================================\n");
    fprintf(stdout, "  \x1b[33mSUS — Unit and Performance Tests\x1b[36m\n");
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
        double performance_test_results[num_tests][3];

        performance_test_names[0] = "wrong";
        performance_test_results[0][0] = measure_and_assert_solver(sus_count_solutions, "examples/sudoku/9x9-wrong.txt", 0);
        performance_test_results[0][1] = measure_and_assert_solver(count_solutions_with_dp_wrapper, "examples/sudoku/9x9-wrong.txt", 0);
        performance_test_results[0][2] = measure_and_assert_solver(sudoku_get_solutions, "examples/sudoku/9x9-wrong.txt", 0);

        performance_test_names[1] = "easy";
        performance_test_results[1][0] = measure_and_assert_solver(sus_count_solutions, "examples/sudoku/9x9-easy.txt", 1);
        performance_test_results[1][1] = measure_and_assert_solver(count_solutions_with_dp_wrapper, "examples/sudoku/9x9-easy.txt", 1);
        performance_test_results[1][2] = measure_and_assert_solver(sudoku_get_solutions, "examples/sudoku/9x9-easy.txt", 1);

        performance_test_names[2] = "medium";
        performance_test_results[2][0] = measure_and_assert_solver(sus_count_solutions, "examples/sudoku/9x9-medium.txt", 1);
        performance_test_results[2][1] = measure_and_assert_solver(count_solutions_with_dp_wrapper, "examples/sudoku/9x9-medium.txt", 1);
        performance_test_results[2][2] = measure_and_assert_solver(sudoku_get_solutions, "examples/sudoku/9x9-medium.txt", 1);

        performance_test_names[3] = "hard";
        performance_test_results[3][0] = measure_and_assert_solver(sus_count_solutions, "examples/sudoku/9x9-hard.txt", 1);
        performance_test_results[3][1] = measure_and_assert_solver(count_solutions_with_dp_wrapper, "examples/sudoku/9x9-hard.txt", 1);
        performance_test_results[3][2] = measure_and_assert_solver(sudoku_get_solutions, "examples/sudoku/9x9-hard.txt", 1);

        performance_test_names[4] = "very hard";
        performance_test_results[4][0] = measure_and_assert_solver(sus_count_solutions, "examples/sudoku/9x9-very-hard.txt", 1);
        performance_test_results[4][1] = measure_and_assert_solver(count_solutions_with_dp_wrapper, "examples/sudoku/9x9-very-hard.txt", 1);
        performance_test_results[4][2] = 0;// measure_and_assert_solver(sudoku_get_solutions, "examples/sudoku/9x9-very-hard.txt", 1);

        performance_test_names[5] = "multiple solutions (2761)";
        performance_test_results[5][0] = measure_and_assert_solver(sus_count_solutions, "examples/sudoku/9x9-multiple-solutions.txt", 2761);
        performance_test_results[5][1] = measure_and_assert_solver(count_solutions_with_dp_wrapper, "examples/sudoku/9x9-multiple-solutions.txt", 2761);
        performance_test_results[5][2] = 0; // measure_and_assert_solver(sudoku_get_solutions, "examples/sudoku/9x9-multiple-solutions.txt", 2761);

        performance_test_names[6] = "even more solutions (>100.000)";
        performance_test_results[6][0] = measure_and_assert_solver(sus_count_solutions, "examples/sudoku/9x9-a-lot-solutions.txt", -1);
        performance_test_results[6][1] = measure_and_assert_solver(count_solutions_with_dp_wrapper, "examples/sudoku/9x9-a-lot-solutions.txt", -1);
        performance_test_results[6][2] = 0; // measure_and_assert_solver(sudoku_get_solutions, "examples/sudoku/9x9-a-lot-solutions.txt", 2761);

        fprintf(stdout, "\nTest name                           SUS                  SUS (no DP)        Brute Force\n");
        for (int i = 0; i < num_tests; i++) {
            print_performance_row(performance_test_names[i],
                performance_test_results[i][0],
                performance_test_results[i][1],
                performance_test_results[i][2]);
        }
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
