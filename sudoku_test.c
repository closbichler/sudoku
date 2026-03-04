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

void sus_print_sets(Matrix sets, SetCover cover)
{
    int sum[sets.items[0].count];
    for (int i = 0; i < sets.items[0].count; i++) {
        sum[i] = 0;
    }

    for (int i = 0; i < sets.count; i++) {
        printf("%03d: ", sets.items[i].id);
        for (int j = 0; j < sets.items[i].count; j++) {
            int val = sets.items[i].items[j];
            if (val == 1) 
                printf("\033[31m%d\033[0m", val);
            else 
                printf("%d", val);
        }
        
        for (int k = 0; k < cover.count; k++) {
            if (sets.items[i].id == cover.items[k]) {
                printf(" +++");
                for (int j = 0; j < sets.items[i].count; j++) {
                    if (sets.items[i].items[j] == 1)
                        sum[j]++;
                }
                break;
            }
        }
        printf("\n");
    }

    printf("sum: ");
    for (int i = 0; i < sets.items[0].count; i++) {
        printf("%d", sum[i]);
    }
    printf("\n");
}

void sus_dlx_print_column(DLXNode *head) 
{
    DLXNode *n = head;
    int current_set = 0;
    do {
        for (int i=0; i<n->set_id - current_set; i++) printf("  ");
        current_set = n->set_id + 1;
        
        printf("%d ", n->val);
        n = n->down;
    } while (n != head);
    printf("\n");
}

void sus_dlx_print_columns(DLXColumn *root)
{
    DLXColumn *c = root->next;
    printf("Col (len): \n");
    do {
        printf("%03d (%02d): ", c->id, c->len);
        if (c->head == NULL) {
            c = c->next;
            printf("\n");
            continue;
        }
        sus_dlx_print_column(c->head);
        c = c->next;
    } while (c != root);
}

// -- Random test --

int pseudorandom(int n) 
{
    return rand() % n;
}

void generate_and_solve_random_sudoku(int hints, int max_attempts) 
{
    fprintf(stdout, "Generating random sudoku with %d hints.\n", hints);
    fprintf(stdout, "attempt ");

    srand(time(0));

    Sudoku s = {0};
    int attempts = 0;
    while (attempts < max_attempts) {
        attempts++;
        fprintf(stdout, "%d..", attempts);
        fflush(stdout);
        s = sudoku_create(9, 3, pseudorandom, hints);
        int solutions = sus_count_solutions(s);
        if (solutions == 1) break;
    }
    if (attempts >= max_attempts) {
        fprintf(stdout, "\nfailed after %d attempts.\n", max_attempts);
        return;
    }

    fprintf(stdout, "it worked.\n");
    sudoku_print(s);
    sus_solve_sudoku(&s);
    sudoku_print(s);   
}

// -- Performance Tests --

double measure_and_assert_solver(uint (*F)(Sudoku), Sudoku s, uint num_solutions) 
{
    clock_t start, end;
    double cpu_time_used;
    uint solutions;

    start = clock();
    solutions = F(s);
    end = clock();
    if (solutions != num_solutions) {
        fprintf(stderr, "\x1b[31m Expected %d solutions but got %d.\x1b[0m\n", num_solutions, solutions);
    }
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    return cpu_time_used;
}

void print_performance_row(const char *name, double a, double b, double c)
{
    const char *color = "\x1b[36m"; // cyan
    const char *reset = "\x1b[0m";
    char s0[64], s1[64], s2[64];

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
    fprintf(stdout, " %s%s%s\n", color, s2, reset);
}

// -- Unit Tests -- 

bool set_covers_equal(SetCover expected_cover, SetCover actual_cover) 
{
    if (expected_cover.count != actual_cover.count) return false;

    for (int i = 0; i < expected_cover.count; i++) {
        bool found = false;
        for (int j=0; j<actual_cover.count; j++) {
            if (expected_cover.items[i] == actual_cover.items[j]) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

void create_empty_sets(int num_sets, int set_size, Matrix *sets)
{
    for (int i = 0; i < num_sets; i++) {
        Set set = (Set){0};
        set.id = i;
        set.count = set_size;
        set.capacity = set_size;
        da_append(sets, set);
    }
}

bool test_delete_set_by_id()
{
    Matrix sets = {0};
    create_empty_sets(3, 3, &sets);
    
    sets.items[0].items = (int[]) { 1,0,0 };
    sets.items[1].items = (int[]) { 0,1,0 };
    sets.items[2].items = (int[]) { 0,0,1 };
    
    bool result = sus_delete_set_by_id(&sets, 1);
    if (!result) return false;
    if (sets.count != 2) return false;
    if (sets.items[0].id != 0) return false;
    if (sets.items[1].id != 2) return false;
    
    result = sus_delete_set_by_id(&sets, 5);
    if (result) return false;
    if (sets.count != 2) return false;
    return true;
}

bool test_matrix_to_linked_list()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = sus_dlx_matrix_to_linked_list(matrix);
    if (root->next->id != 0) return false;
    if (root->next->len != 2) return false;
    if (root->next->next->id != 1) return false;
    if (root->next->next->len != 2) return false;
    if (root->next->next->next->id != 2) return false;
    if (root->next->next->next->len != 1) return false;
    if (root->next->next->next->next != root) return false;
    
    DLXNode *n = root->next->head;
    if (n->val != 1) return false;
    if (n->set_id != 0) return false;
    if (n->down->val != 4) return false;
    if (n->down->set_id != 2) return false;
    if (n->down->down != root->next->head) return false;
    if (n->right->val != 2) return false;
    if (n->right->set_id != 0) return false;
    return true;
}

bool test_remove_node() 
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = sus_dlx_matrix_to_linked_list(matrix);

    sus_dlx_remove_node(root->next->head);
    if (root->next->len != 1) return false;
    if (root->next->head->set_id != 2) return false;
    if (root->next->head->down != root->next->head) return false;
    if (root->next->head->right != root->next->next->head->down) return false;

    sus_dlx_remove_node(root->next->next->next->head);
    if (root->next->next->next->len != 0) return false;
    if (root->next->next->next->head != NULL) return false;
    return true;
}

bool test_unremove_node()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = sus_dlx_matrix_to_linked_list(matrix);

    DLXNode *n = root->next->head;
    sus_dlx_remove_node(root->next->head);

    sus_dlx_unremove_node(n);
    if (root->next->len != 2) return false;
    if (root->next->head != n) return false;
    if (root->next->head->set_id != 0) return false;
    return true;
}

bool test_cover_column()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = sus_dlx_matrix_to_linked_list(matrix);

    sus_dlx_cover_column(root->next->next->next);
    if (root->next->next->next != root) return false;
    if (root->next->len != 1) return false;
    if (root->next->head->val != 4) return false;
    if (root->next->head->set_id != 2) return false;

    sus_dlx_cover_column(root->next);
    if (root->next->len != 1) return false;
    if (root->next->head->val != 3) return false;
    return true;
}

bool test_uncover_column_1() 
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = sus_dlx_matrix_to_linked_list(matrix);
    DLXColumn *c = root->next->next->next;
    sus_dlx_cover_column(c);

    sus_dlx_uncover_column(c);
    if (root->next->next->next != c) return false;
    
    c = root->next;
    sus_dlx_cover_column(c);
    
    sus_dlx_uncover_column(c);  
    if (root->next != c) return false;
    if (root->next->next->head->down->left->val != 4) return false;
    return true;
}

bool test_uncover_column_2() 
{
    Matrix sets = {0};
    create_empty_sets(6, 7, &sets);

    sets.items[0].items = (int[]) { 1,0,0,1,0,0,1 };
    sets.items[1].items = (int[]) { 1,0,0,1,0,0,0 };
    sets.items[2].items = (int[]) { 0,0,0,1,1,0,1 };
    sets.items[3].items = (int[]) { 0,0,1,0,1,1,0 };
    sets.items[4].items = (int[]) { 0,1,1,0,0,1,1 };
    sets.items[5].items = (int[]) { 0,1,0,0,0,0,1 };

    DLXColumn *root = sus_dlx_matrix_to_linked_list(sets);
    DLXColumn *c = root->next;
    sus_dlx_cover_column(c);

    sus_dlx_uncover_column(c);
    if (root->next != c) return false;
    if (root->next->next->len != 2) return false;
    return true;
}

bool test_get_shortest_column()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);
    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = sus_dlx_matrix_to_linked_list(matrix);
    DLXColumn *c = sus_dlx_get_shortest_column(root);
    if (c->id != 2) return false;
    if (c != root->next->next->next) return false;

    matrix = (Matrix) {0};
    create_empty_sets(6, 7, &matrix);
    matrix.items[0].items = (int[]) { 1,0,0,1,0,0,1 };
    matrix.items[1].items = (int[]) { 1,0,0,1,0,0,0 };
    matrix.items[2].items = (int[]) { 0,0,0,1,1,0,1 };
    matrix.items[3].items = (int[]) { 0,0,1,0,1,1,0 };
    matrix.items[4].items = (int[]) { 0,1,1,0,0,1,1 };
    matrix.items[5].items = (int[]) { 0,1,0,0,0,0,1 };

    root = sus_dlx_matrix_to_linked_list(matrix);
    c = sus_dlx_get_shortest_column(root);
    if (c->id != 0) return false;
    if (c != root->next) return false;
    return true;
}

bool test_solve_exact_cover_1(SetCover expected_cover)
{
    Matrix sets = {0};
    create_empty_sets(3, 3, &sets);

    sets.items[0].items = (int[]) { 1,0,1 };
    sets.items[1].items = (int[]) { 1,1,0 };
    sets.items[2].items = (int[]) { 0,1,0 };
    
    SetCover cover = {0};
    DLXColumn *root = sus_dlx_matrix_to_linked_list(sets);

    int result = sus_dlx_solve_exact_cover(root, &cover, true);
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_2(SetCover expected_cover)
{
    Matrix sets = {0};
    create_empty_sets(6, 7, &sets);

    sets.items[0].items = (int[]) { 1,0,0,1,0,0,1 };
    sets.items[1].items = (int[]) { 1,0,0,1,0,0,0 };
    sets.items[2].items = (int[]) { 0,0,0,1,1,0,1 };
    sets.items[3].items = (int[]) { 0,0,1,0,1,1,0 };
    sets.items[4].items = (int[]) { 0,1,1,0,0,1,1 };
    sets.items[5].items = (int[]) { 0,1,0,0,0,0,1 };
    
    SetCover cover = {0};
    DLXColumn *root = sus_dlx_matrix_to_linked_list(sets);

    int result = sus_dlx_solve_exact_cover(root, &cover, true);
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_3(SetCover expected_cover)
{
    Matrix sets = {0};
    create_empty_sets(6, 7, &sets);

    sets.items[0].items = (int[]) { 0,1,0,0,1,0,0 };
    sets.items[1].items = (int[]) { 0,0,1,0,0,1,0 };
    sets.items[2].items = (int[]) { 1,0,0,0,0,0,0 };
    sets.items[3].items = (int[]) { 0,0,0,1,1,0,1 };
    sets.items[4].items = (int[]) { 0,1,1,0,0,1,0 };
    sets.items[5].items = (int[]) { 0,0,0,0,0,0,0 };
    
    SetCover cover = {0};
    DLXColumn *root = sus_dlx_matrix_to_linked_list(sets);

    int result = sus_dlx_solve_exact_cover(root, &cover, true);
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_incomplete(SetCover expected_cover)
{
    Matrix sets = {0};
    create_empty_sets(6, 7, &sets);

    sets.items[0].items = (int[]) { 0,1,0,0,1,0,0 };
    sets.items[1].items = (int[]) { 0,0,1,0,0,1,0 };
    sets.items[2].items = (int[]) { 1,0,0,1,0,0,0 };
    sets.items[3].items = (int[]) { 0,0,0,1,1,0,1 };
    sets.items[4].items = (int[]) { 0,1,1,0,0,1,0 };
    sets.items[5].items = (int[]) { 0,0,0,0,0,0,0 };
    
    SetCover cover = {0};
    DLXColumn *root = sus_dlx_matrix_to_linked_list(sets);

    int result = sus_dlx_solve_exact_cover(root, &cover, true);
    if (result != 0) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

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
    int result = sus_count_solutions(s2);
    if(result != 2761) return false;

    Sudoku s3 = {0};
    sudoku_example_no_solutions(&s3);
    if(sus_count_solutions(s3) != 0) return false;
    return true;
}

typedef struct {
    int* items;
    size_t capacity;
    size_t count;
} TestArr;

int main(int argc, char *argv[])
{
    bool test_unit = true;
    bool test_performance = true;
    bool test_random = argc > 1;

    fprintf(stdout, "Unit test summary:\n");
    if (!test_unit) {
        fprintf(stdout, "skipped.\n\n");
    } else {
        int num_tests = 15;
        char* test_names[num_tests];
        bool test_results[num_tests];

        test_names[0] = "delete_set_by_id";
        test_results[0] = test_delete_set_by_id();
        test_names[1] = "matrix_to_linked_list";
        test_results[1] = test_matrix_to_linked_list();
        test_names[2] = "remove_node";
        test_results[2] = test_remove_node();
        test_names[3] = "unremove_node";
        test_results[3] = test_unremove_node();
        test_names[4] = "cover_column";
        test_results[4] = test_cover_column();
        test_names[5] = "uncover_column_1";
        test_results[5] = test_uncover_column_1();
        test_names[6] = "uncover_column_2";
        test_results[6] = test_uncover_column_2();
        test_names[7] = "get_shortest_column";
        test_results[7] = test_get_shortest_column();
        test_names[8] = "solve_exact_cover_1";
        test_results[8] = test_solve_exact_cover_1((SetCover){.items=(int[]){0,2}, .count=2});
        test_names[9] = "solve_exact_cover_2";
        test_results[9] = test_solve_exact_cover_2((SetCover){.items=(int[]){1,3,5}, .count=3});
        test_names[10] = "solve_exact_cover_3";
        test_results[10] = test_solve_exact_cover_3((SetCover){.items=(int[]){2,3,4}, .count=3});
        test_names[11] = "solve_exact_cover_incomplete";
        test_results[11] = test_solve_exact_cover_incomplete((SetCover){.items=(int[]){}, .count=0});
        test_names[12] = "solve_sudoku_1";
        test_results[12] = test_solve_sudoku_1();
        printf("kek\n");
        test_names[13] = "solve_sudoku_2";
        test_results[13] = test_solve_sudoku_2();
        test_names[14] = "count_solutions";
        test_results[14] = test_count_solutions();

        // Print test results with dot-fill and colored PASS/FAIL
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
        Sudoku s = {0};

        sudoku_example_wrong(&s);        
        performance_test_names[0] = "wrong";
        performance_test_results[0][0] = measure_and_assert_solver(sus_count_solutions, s, 0);
        performance_test_results[0][1] = measure_and_assert_solver(sus_count_solutions_legacy, s, 0);
        performance_test_results[0][2] = measure_and_assert_solver(sudoku_get_solutions, s, 0);

        sudoku_example_easy(&s);
        performance_test_names[1] = "easy";
        performance_test_results[1][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[1][1] = measure_and_assert_solver(sus_count_solutions_legacy, s, 1);
        performance_test_results[1][2] = measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_medium(&s);
        performance_test_names[2] = "medium";
        performance_test_results[2][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[2][1] = measure_and_assert_solver(sus_count_solutions_legacy, s, 1);
        performance_test_results[2][2] = measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_hard(&s);
        performance_test_names[3] = "hard";
        performance_test_results[3][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[3][1] = measure_and_assert_solver(sus_count_solutions_legacy, s, 1);
        performance_test_results[3][2] = measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_very_hard(&s);
        performance_test_names[4] = "very hard";
        performance_test_results[4][0] = measure_and_assert_solver(sus_count_solutions, s, 1);
        performance_test_results[4][1] = measure_and_assert_solver(sus_count_solutions_legacy, s, 1);
        performance_test_results[4][2] = 0;// measure_and_assert_solver(sudoku_get_solutions, s, 1);

        sudoku_example_multiple_solutions(&s);
        performance_test_names[5] = "multiple solutions";
        performance_test_results[5][0] = measure_and_assert_solver(sus_count_solutions, s, 2761);
        performance_test_results[5][1] = measure_and_assert_solver(sus_count_solutions_legacy, s, 2761);
        performance_test_results[5][2] = 0; // measure_and_assert_solver(sudoku_get_solutions, s, 2761);

        sudoku_example_even_more_solutions(&s);
        performance_test_names[6] = "even more solutions";
        performance_test_results[6][0] = measure_and_assert_solver(sus_count_solutions, s, 2761);
        performance_test_results[6][1] = 0; // measure_and_assert_solver(sus_count_solutions_legacy, s, 2761);
        performance_test_results[6][2] = 0; // measure_and_assert_solver(sudoku_get_solutions, s, 2761);

        fprintf(stdout, "\nTest name                           Exact Cover w DLX    Exact Cover w/o DLX    Brute Force\n");
        for (int i = 0; i < num_tests; i++) {
            print_performance_row(performance_test_names[i],
                performance_test_results[i][0],
                performance_test_results[i][1],
                performance_test_results[i][2]);
        }
        fprintf(stdout, "\n\n");
    }
  
    fprintf(stdout, "Random sudoku test: \n");
    if (!test_random) {
        fprintf(stdout, "skipped.\n\n");
    } else {
        fprintf(stdout, "\n");
        generate_and_solve_random_sudoku(19, 30);
        fprintf(stdout, "\n\n");
    }

    return 0;
}