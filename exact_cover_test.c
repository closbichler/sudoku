#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>

#define EXACT_COVER_IMPLEMENTATION
#include "exact_cover.h"

// -- Print functions --

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

    int pos1 = 48, pos2 = 65, pos3 = 79;
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

// -- Helper functions --

bool set_covers_equal(SetCover expected_cover, SetCover actual_cover) 
{
    if (expected_cover.count != actual_cover.count) return false;

    for (size_t i = 0; i < expected_cover.count; i++) {
        bool found = false;
        for (size_t j=0; j<actual_cover.count; j++) {
            if (expected_cover.items[i] == actual_cover.items[j]) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

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

double measure_and_assert_solve_constraints(const char *filename, unsigned long max_solutions, unsigned long num_solutions, bool with_dp) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "\x1b[31m Failed to open file %s.\x1b[0m\n", filename);
        return 0;
    }

    uint8_t **constraints;
    long rows, cols;
    if (!parse_constraints(file, &constraints, &rows, &cols)) {
        fprintf(stderr, "\x1b[31m Failed to parse constraints from file %s.\x1b[0m\n", filename);
        fclose(file);
        return 0;
    }
    fclose(file);

    clock_t start, end;
    double cpu_time_used;
    SetCover cover = {0};
    ExactCoverProblem result;

    start = clock();
    if (with_dp) {
        result = exact_solve_constraints_with_dp(constraints, rows, cols, &cover, 0, max_solutions);
    } else {
        result = exact_solve_constraints(constraints, rows, cols, &cover, 0, max_solutions);
    }
    end = clock();
    
    if (num_solutions != -1UL && result.solutions != num_solutions) {
        fprintf(stderr, "\x1b[31m Expected %ld solutions but got %ld.\x1b[0m\n", num_solutions, result.solutions);
    }
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    return cpu_time_used;
}

// -- Unit tests --

bool test_exact_constraints_to_dlx()
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,2 };
    constraint_sets[1] = (uint8_t[]) { 0,3,0 };
    constraint_sets[2] = (uint8_t[]) { 4,5,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);
    
    if (root == NULL) return false;
    if (root->next == root) return false;
    if (root->next->id != 0) return false;
    if (root->next->next->id != 1) return false;
    if (root->next->next->next->id != 2) return false;
    if (root->next->next->next->next != root) return false;
    if (root->next->next->next->len != 1) return false;
    if (root->next->next->next->head->val != 2) return false;
    if (root->next->next->head->down->val != 5) return false;

    return true;
}

bool test_remove_node() 
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,2 };
    constraint_sets[1] = (uint8_t[]) { 0,3,0 };
    constraint_sets[2] = (uint8_t[]) { 4,5,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);

    exact_remove_node(root->next->head);
    if (root->next->len != 1) return false;
    if (root->next->head->set_id != 2) return false;
    if (root->next->head->down != root->next->head) return false;
    if (root->next->head->right != root->next->next->head->down) return false;

    exact_remove_node(root->next->next->next->head);
    if (root->next->next->next->len != 0) return false;
    if (root->next->next->next->head != NULL) return false;
    return true;
}

bool test_unremove_node()
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,2 };
    constraint_sets[1] = (uint8_t[]) { 0,3,0 };
    constraint_sets[2] = (uint8_t[]) { 4,5,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);

    DLXNode *n = root->next->head;
    exact_remove_node(root->next->head);

    exact_unremove_node(n);
    if (root->next->len != 2) return false;
    if (root->next->head != n) return false;
    if (root->next->head->set_id != 0) return false;
    return true;
}

bool test_cover_column()
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,2 };
    constraint_sets[1] = (uint8_t[]) { 0,3,0 };
    constraint_sets[2] = (uint8_t[]) { 4,5,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);

    exact_cover_column(root->next->next->next);
    if (root->next->next->next != root) return false;
    if (root->next->len != 1) return false;
    if (root->next->head->val != 4) return false;
    if (root->next->head->set_id != 2) return false;

    exact_cover_column(root->next);
    if (root->next->len != 1) return false;
    if (root->next->head->val != 3) return false;
    return true;
}

bool test_uncover_column_1() 
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,2 };
    constraint_sets[1] = (uint8_t[]) { 0,3,0 };
    constraint_sets[2] = (uint8_t[]) { 4,5,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);
    DLXColumn *c = root->next->next->next;
    exact_cover_column(c);

    exact_uncover_column(c);
    if (root->next->next->next != c) return false;
    
    c = root->next;
    exact_cover_column(c);
    
    exact_uncover_column(c);  
    if (root->next != c) return false;
    if (root->next->next->head->down->left->val != 4) return false;
    return true;
}

bool test_uncover_column_2() 
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(6, 7);

    constraint_sets[0] = (uint8_t[]) { 1,0,0,1,0,0,1 };
    constraint_sets[1] = (uint8_t[]) { 1,0,0,1,0,0,0 };
    constraint_sets[2] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[3] = (uint8_t[]) { 0,0,1,0,1,1,0 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,1 };
    constraint_sets[5] = (uint8_t[]) { 0,1,0,0,0,0,1 };

    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    DLXColumn *c = root->next;
    exact_cover_column(c);

    exact_uncover_column(c);
    if (root->next != c) return false;
    if (root->next->next->len != 2) return false;
    return true;
}

bool test_get_shortest_column()
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,2 };
    constraint_sets[1] = (uint8_t[]) { 0,3,0 };
    constraint_sets[2] = (uint8_t[]) { 4,5,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);
    DLXColumn *c = exact_get_shortest_column(root);
    if (c->id != 2) return false;
    if (c != root->next->next->next) return false;

    constraint_sets = exact_create_empty_constraint_sets(6, 7);
    constraint_sets[0] = (uint8_t[]) { 1,0,0,1,0,0,1 };
    constraint_sets[1] = (uint8_t[]) { 1,0,0,1,0,0,0 };
    constraint_sets[2] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[3] = (uint8_t[]) { 0,0,1,0,1,1,0 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,1 };
    constraint_sets[5] = (uint8_t[]) { 0,1,0,0,0,0,1 };

    root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    c = exact_get_shortest_column(root);
    if (c->id != 0) return false;
    if (c != root->next) return false;
    return true;
}

bool test_solve_exact_cover_1(SetCover expected_cover)
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,1 };
    constraint_sets[1] = (uint8_t[]) { 1,1,0 };
    constraint_sets[2] = (uint8_t[]) { 0,1,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);
    SetCover cover = {0};
    
    exact_create_setcover_hashtable(16);
    int result = exact_solve(root, &cover, true, -1).solutions;
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_2(SetCover expected_cover)
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(6, 7);
    constraint_sets[0] = (uint8_t[]) { 1,0,0,1,0,0,1 };
    constraint_sets[1] = (uint8_t[]) { 1,0,0,1,0,0,0 };
    constraint_sets[2] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[3] = (uint8_t[]) { 0,0,1,0,1,1,0 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,1 };
    constraint_sets[5] = (uint8_t[]) { 0,1,0,0,0,0,1 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    SetCover cover = {0};

    exact_create_setcover_hashtable(16);
    int result = exact_solve(root, &cover, true, -1).solutions;
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_3(SetCover expected_cover)
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(6, 7);
    constraint_sets[0] = (uint8_t[]) { 0,1,0,0,1,0,0 };
    constraint_sets[1] = (uint8_t[]) { 0,0,1,0,0,1,0 };
    constraint_sets[2] = (uint8_t[]) { 1,0,0,0,0,0,0 };
    constraint_sets[3] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,0 };
    constraint_sets[5] = (uint8_t[]) { 0,0,0,0,0,0,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    SetCover cover = {0};

    int result = exact_solve(root, &cover, true, -1).solutions;
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_incomplete(SetCover expected_cover)
{
    uint8_t **constraint_sets = exact_create_empty_constraint_sets(6, 7);
    constraint_sets[0] = (uint8_t[]) { 0,1,0,0,1,0,0 };
    constraint_sets[1] = (uint8_t[]) { 0,0,1,0,0,1,0 };
    constraint_sets[2] = (uint8_t[]) { 1,0,0,1,0,0,0 };
    constraint_sets[3] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,0 };
    constraint_sets[5] = (uint8_t[]) { 0,0,0,0,0,0,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    SetCover cover = {0};

    exact_create_setcover_hashtable(16);
    int result = exact_solve(root, &cover, true, -1).solutions;
    if (result != 0) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

int main() 
{
    bool test_unit = true;
    bool test_performance = true;

    fprintf(stdout, "\n\x1b[36m\x1b[1m========================================\n");
    fprintf(stdout, "  \x1b[33mExact Cover — Unit and Performance Tests\x1b[36m\n");
    fprintf(stdout, "========================================\x1b[0m\n\n");

    fprintf(stdout, "Unit test summary:\n");
    if (!test_unit) {
        fprintf(stdout, "skipped.\n\n");
    } else {
        int num_tests = 12;
        char* test_names[num_tests];
        bool test_results[num_tests];
        
        test_names[0] = "test_exact_constraints_to_dlx";
        test_results[0] = test_exact_constraints_to_dlx();
        test_names[1] = "remove_node";
        test_results[1] = test_remove_node();
        test_names[2] = "unremove_node";
        test_results[2] = test_unremove_node();
        test_names[3] = "cover_column";
        test_results[3] = test_cover_column();
        test_names[4] = "uncover_column_1";
        test_results[4] = test_uncover_column_1();
        test_names[5] = "uncover_column_2";
        test_results[5] = test_uncover_column_2();
        test_names[6] = "get_shortest_column";
        test_results[6] = test_get_shortest_column();
        test_names[7] = "";
        test_results[7] = true;
        test_names[8] = "solve_exact_cover_1";
        test_results[8] = test_solve_exact_cover_1((SetCover){.items=(int[]){0,2}, .count=2});
        test_names[9] = "solve_exact_cover_2";
        test_results[9] = test_solve_exact_cover_2((SetCover){.items=(int[]){1,3,5}, .count=3});
        test_names[10] = "solve_exact_cover_3";
        test_results[10] = test_solve_exact_cover_3((SetCover){.items=(int[]){2,3,4}, .count=3});
        test_names[11] = "solve_exact_cover_incomplete";
        test_results[11] = test_solve_exact_cover_incomplete((SetCover){.items=(int[]){}, .count=0});
    
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
        int num_tests = 8;
        char *performance_test_names[num_tests];
        double performance_test_results[num_tests][2];

        performance_test_names [0] = "4-2 (2 solutions)";
        performance_test_results[0][0] = measure_and_assert_solve_constraints("examples/exact_cover/4-2.txt", 1000, 2, false);
        performance_test_results[0][1] = measure_and_assert_solve_constraints("examples/exact_cover/4-2.txt", 1000, 2, true);
        
        performance_test_names[1] = "30-10 (118 solutions)";
        performance_test_results[1][0] = measure_and_assert_solve_constraints("examples/exact_cover/30-10.txt", 1000, 118, false);
        performance_test_results[1][1] = measure_and_assert_solve_constraints("examples/exact_cover/30-10.txt", 1000, 118, true);
       
        performance_test_names[2] = "empty_4x4_sudoku (288 solutions)";
        performance_test_results[2][0] = measure_and_assert_solve_constraints("examples/exact_cover/empty_4x4_sudoku.txt", 1000, 288, false);
        performance_test_results[2][1] = measure_and_assert_solve_constraints("examples/exact_cover/empty_4x4_sudoku.txt", 1000, 288, true);

        performance_test_names[3] = "empty_9x9_sudoku (1000+ solutions)";
        performance_test_results[3][0] = measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 1000, -1, false);
        performance_test_results[3][1] = measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 1000, -1, true);

        performance_test_names[4] = "empty_9x9_sudoku (10000+ solutions)";
        performance_test_results[4][0] = measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 10000, -1, false);
        performance_test_results[4][1] = measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 10000, -1, true);

        performance_test_names[5] = "empty_9x9_sudoku (100000+ solutions)";
        performance_test_results[5][0] = measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 100000, -1, false);
        performance_test_results[5][1] = 0; //measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 100000, -1, true);

        performance_test_names[5] = "empty_9x9_sudoku (1000000+ solutions)";
        performance_test_results[5][0] = measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 1000000, -1, false);
        performance_test_results[5][1] = 0; // measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 1000000, -1, true);

        performance_test_names[6] = "empty_9x9_sudoku (2000000+ solutions)";
        performance_test_results[6][0] = measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 2000000, -1, false);
        performance_test_results[6][1] = 0; //measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 2000000, -1, true);

        performance_test_names[7] = "empty_9x9_sudoku (3000000+ solutions)";
        performance_test_results[7][0] = 0; // measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 3000000, -1, false);
        performance_test_results[7][1] = 0; // measure_and_assert_solve_constraints("examples/exact_cover/empty_9x9_sudoku.txt", 3000000, -1, true);

        fprintf(stdout, "\nTest name                                        without DP          with DP        \n");
        for (int i = 0; i < num_tests; i++) {
            print_performance_row(performance_test_names[i],
                performance_test_results[i][0],
                performance_test_results[i][1],
                0);
        }
        fprintf(stdout, "\n\n");
    }

    return 0;
}