#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#define EXACT_COVER_IMPLEMENTATION
#include "exact_cover.h"

bool parse_constraints(FILE *file, uint8_t ***constraints, long *num_rows, long *num_cols)
{
    char line[4096];
    fgets(line, sizeof(line), file);

    if (strncmp(line, "exact_cover ", 12) != 0) {
        fprintf(stderr, "\x1b[31m Invalid file format: expected line starting with 'exact_cover '.\x1b[0m\n");
        return false;
    }

    char *endptr;
    *num_rows = strtol(line + 12, &endptr, 10);
    if (*num_rows < 0 || endptr == line || errno != 0 || *num_rows == LONG_MIN || *num_rows == LONG_MAX) {
        fprintf(stderr, "\x1b[31m Invalid number of rows: %ld.\x1b[0m\n", *num_rows);
        return false;
    }
    *num_cols = strtol(endptr + 1, &endptr, 10);
    if (*num_cols < 0 || endptr == line || errno != 0 || *num_cols == LONG_MIN || *num_cols == LONG_MAX) {
        fprintf(stderr, "\x1b[31m Invalid number of columns: %ld.\x1b[0m\n", *num_cols);
        return false;
    }

    *constraints = exact_create_empty_constraint_sets(*num_rows, *num_cols);
    for (int i = 0; i < *num_rows; i++) {
        fgets(line, sizeof(line), file);
        char *p = line;
        for (int j = 0; j < *num_cols; j++) {
            int val = (int)strtol(p, &p, 10);
            (*constraints)[i][j] = (uint8_t)val;
        }
    }
    
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    unsigned long max_solutions = 5000000;
    bool diagnostic_output = true;
    bool print_solution = true;

    clock_t start, end;
    double parse_time, cpu_time;

    start = clock();
    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        return 1;
    }
    uint8_t **constraints;
    long num_rows, num_cols;
    if (!parse_constraints(input_file, &constraints, &num_rows, &num_cols)) {
        fprintf(stderr, "Error: Failed to parse constraints from file %s\n", argv[1]);
        fclose(input_file);
        return 1;
    }
    fclose(input_file);
    end = clock();
    parse_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    SetCover cover = {0};
    start = clock();
    ExactCoverProblem result = exact_solve_constraints(constraints, num_rows, num_cols, &cover, false, max_solutions);
    end = clock();
    cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    if (!diagnostic_output) {
        printf("%lu\n", result.solutions);
    } else {
        char parse_time_str[32];
        char cpu_time_str[32];
        char algo_info_str[256];
        snprintf(parse_time_str, sizeof(parse_time_str), "%.2f s", parse_time);
        snprintf(cpu_time_str, sizeof(cpu_time_str), "%.2f s", cpu_time);
        snprintf(algo_info_str, sizeof(algo_info_str), "Removed duplicates during preprocessing: %d", result.preprocessing_removed_duplicates);

        printf("============================[ Problem Statistics ]=============================\n");
        printf("| %-30s %-16ld %-27s |\n", "Number of sets:", num_rows, "");
        printf("| %-30s %-16ld %-27s |\n", "Number of elements:", num_cols, "");
        printf("| %-30s %-16s %-27s |\n", "Parse time:", parse_time_str, "");
        printf("| %-30s %-16s %-27s |\n", "CPU time:", cpu_time_str, "");
        printf("============================[      Algorithm     ]=============================\n");
        printf("| %-75s |\n", algo_info_str);
        printf("===============================================================================\n");
        printf("\n");

        if (result.solutions == 0)                  printf("NO SOLUTIONS\n");
        else if (result.solutions >= max_solutions) printf(">%lu SOLUTIONS (LIMIT REACHED)\n", max_solutions);
        else                                         printf("%lu SOLUTIONS\n", result.solutions);
    }
    
    if (print_solution && result.solutions > 0) {
        printf("\nFirst solution:\n");

        cover = (SetCover){0};
        exact_solve_constraints(constraints, num_rows, num_cols, &cover, true, 1);

        for (unsigned long i = 0; i < cover.count; i++) {
            printf("%d ", cover.items[i]);    
        }
        printf("\n");
    }

    return 0;
}