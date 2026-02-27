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

void measure_and_assert_solvers(const char* name, Sudoku s, int num_solutions, bool skip_brute_force) 
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
    
    printf("Exact-cover (dlx): ");
    start = clock();
    solutions = solver_count_solutions(s);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
    assert(solutions == num_solutions);
    
    if (skip_brute_force) {
        printf("skipping brute-force solver for this test case\n\n");
        return;
    }
    printf("Brute-force:       ");
    start = clock();
    solutions = sudoku_get_solutions(s);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
    assert(solutions == num_solutions);

    printf("\n");
}

void test_solvers()
{
    Sudoku s = {0};
    sudoku_example_wrong(&s);
    measure_and_assert_solvers("wrong", s, 0, false);

    sudoku_example_easy(&s);
    measure_and_assert_solvers("easy", s, 1, false);
    
    sudoku_example_medium(&s);
    measure_and_assert_solvers("medium", s, 1, false);
    
    sudoku_example_hard(&s);
    measure_and_assert_solvers("hard", s, 1, false);

    sudoku_example_very_hard(&s);
    measure_and_assert_solvers("very hard", s, 1, true);
    
    sudoku_example_multiple_solutions(&s);
    measure_and_assert_solvers("multiple solutions", s, 2761, true);

    sudoku_example_no_solutions(&s);
    measure_and_assert_solvers("no solutions (unsolvable)", s, 0, false);
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

/* Unit Tests */
void assert_cover_eq(SetCover expected_cover, SetCover actual_cover) 
{
    assert(expected_cover.count == actual_cover.count);
    for (int i = 0; i < expected_cover.count; i++) {
        bool found = false;
        for (int j=0; j<actual_cover.count; j++) {
            if (expected_cover.items[i] == actual_cover.items[j]) {
                found = true;
                break;
            }
        }
        assert(found);
    }
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

void test_delete_set_by_id()
{
    Matrix sets = {0};
    create_empty_sets(3, 3, &sets);

    sets.items[0].items = (int[]) { 1,0,0 };
    sets.items[1].items = (int[]) { 0,1,0 };
    sets.items[2].items = (int[]) { 0,0,1 };

    bool result = exact_cover_delete_set_by_id(&sets, 1);
    assert(result);
    assert(sets.count == 2);
    assert(sets.items[0].id == 0);
    assert(sets.items[1].id == 2);

    result = exact_cover_delete_set_by_id(&sets, 5);
    assert(!result);
    assert(sets.count == 2);
}

void test_matrix_to_linked_list()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = dlx_matrix_to_linked_list(matrix);
    assert(root->next->id == 0);
    assert(root->next->len == 2);
    assert(root->next->next->id == 1);
    assert(root->next->next->len == 2);
    assert(root->next->next->next->id == 2);
    assert(root->next->next->next->len == 1);
    assert(root->next->next->next->next == root);
    
    DLXNode *n = root->next->head;
    assert(n->val == 1);
    assert(n->set_id == 0);
    assert(n->down->val == 4);
    assert(n->down->set_id == 2);
    assert(n->down->down == root->next->head);
    assert(n->right->val == 2);
    assert(n->right->set_id == 0);
}

void test_remove_node() 
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = dlx_matrix_to_linked_list(matrix);

    dlx_remove_node(root->next->head);
    assert(root->next->len == 1);
    assert(root->next->head->set_id == 2);
    assert(root->next->head->down == root->next->head);
    assert(root->next->head->right == root->next->next->head->down);

    dlx_remove_node(root->next->next->next->head);
    assert(root->next->next->next->len == 0);
    assert(root->next->next->next->head == NULL);
}

void test_unremove_node()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = dlx_matrix_to_linked_list(matrix);

    DLXNode *n = root->next->head;
    dlx_remove_node(root->next->head);

    dlx_unremove_node(n);
    assert(root->next->len == 2);
    assert(root->next->head == n);
    assert(root->next->head->set_id == 0);
}

void test_cover_column()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = dlx_matrix_to_linked_list(matrix);

    dlx_cover_column(root->next->next->next);
    assert(root->next->next->next == root);
    assert(root->next->len == 1);
    assert(root->next->head->val == 4);
    assert(root->next->head->set_id == 2);

    dlx_cover_column(root->next);
    assert(root->next->len == 1);
    assert(root->next->head->val == 3);
}

void test_uncover_column_1() 
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);

    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = dlx_matrix_to_linked_list(matrix);
    DLXColumn *c = root->next->next->next;
    dlx_cover_column(c);

    dlx_uncover_column(c);
    assert(root->next->next->next == c);
    
    c = root->next;
    dlx_cover_column(c);
    
    dlx_uncover_column(c);  
    assert(root->next == c);
    assert(root->next->next->head->down->left->val == 4);
}

void test_uncover_column_2() 
{
    Matrix sets = {0};
    create_empty_sets(6, 7, &sets);

    sets.items[0].items = (int[]) { 1,0,0,1,0,0,1 };
    sets.items[1].items = (int[]) { 1,0,0,1,0,0,0 };
    sets.items[2].items = (int[]) { 0,0,0,1,1,0,1 };
    sets.items[3].items = (int[]) { 0,0,1,0,1,1,0 };
    sets.items[4].items = (int[]) { 0,1,1,0,0,1,1 };
    sets.items[5].items = (int[]) { 0,1,0,0,0,0,1 };

    DLXColumn *root = dlx_matrix_to_linked_list(sets);
    DLXColumn *c = root->next;
    dlx_cover_column(c);

    dlx_uncover_column(c);
    assert(root->next == c);
    assert(root->next->next->len == 2);
}

void test_get_shortest_column()
{
    Matrix matrix = {0};
    create_empty_sets(3, 3, &matrix);
    matrix.items[0].items = (int[]) { 1,0,2 };
    matrix.items[1].items = (int[]) { 0,3,0 };
    matrix.items[2].items = (int[]) { 4,5,0 };

    DLXColumn *root = dlx_matrix_to_linked_list(matrix);
    DLXColumn *c = dlx_get_shortest_column(root);
    assert(c->id == 2);
    assert(c == root->next->next->next);

    matrix = (Matrix) {0};
    create_empty_sets(6, 7, &matrix);
    matrix.items[0].items = (int[]) { 1,0,0,1,0,0,1 };
    matrix.items[1].items = (int[]) { 1,0,0,1,0,0,0 };
    matrix.items[2].items = (int[]) { 0,0,0,1,1,0,1 };
    matrix.items[3].items = (int[]) { 0,0,1,0,1,1,0 };
    matrix.items[4].items = (int[]) { 0,1,1,0,0,1,1 };
    matrix.items[5].items = (int[]) { 0,1,0,0,0,0,1 };

    root = dlx_matrix_to_linked_list(matrix);
    c = dlx_get_shortest_column(root);
    assert(c->id == 0);
    assert(c == root->next);
}

void test_solve_exact_cover_1(SetCover expected_cover)
{
    Matrix sets = {0};
    create_empty_sets(3, 3, &sets);

    sets.items[0].items = (int[]) { 1,0,1 };
    sets.items[1].items = (int[]) { 1,1,0 };
    sets.items[2].items = (int[]) { 0,1,0 };
    
    SetCover cover = {0};
    DLXColumn *root = dlx_matrix_to_linked_list(sets);

    int result = dlx_solve_exact_cover(root, &cover, true);
    assert(result == 1);
    assert_cover_eq(expected_cover, cover);
}

void test_solve_exact_cover_2(SetCover expected_cover)
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
    DLXColumn *root = dlx_matrix_to_linked_list(sets);

    int result = dlx_solve_exact_cover(root, &cover, true);
    assert(result == 1);
    assert_cover_eq(expected_cover, cover);
}

void test_solve_exact_cover_3(SetCover expected_cover)
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
    DLXColumn *root = dlx_matrix_to_linked_list(sets);

    int result = dlx_solve_exact_cover(root, &cover, true);
    assert(result == 1);
    assert_cover_eq(expected_cover, cover);
}

void test_solve_exact_cover_incomplete(SetCover expected_cover)
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
    DLXColumn *root = dlx_matrix_to_linked_list(sets);

    int result = dlx_solve_exact_cover(root, &cover, true);
    assert(result == 0);
    assert_cover_eq(expected_cover, cover);
}

void test_solve_sudoku_1() 
{
    Sudoku s = {0};
    sudoku_example_easy(&s);
    assert(solver_solve_sudoku(&s));
    assert(sudoku_is_valid(s));
}

void test_solve_sudoku_2() 
{
    Sudoku s = {0};
    sudoku_example_medium(&s);
    assert(solver_solve_sudoku(&s));
    assert(sudoku_is_valid(s));
}

void test_count_solutions()
{
    Sudoku s = {0};
    sudoku_example_multiple_solutions(&s);
    assert(solver_count_solutions(s) == 2761);
}

int main() 
{
    fprintf(stdout, "delete_set_by_id:\n");
    test_delete_set_by_id();

    fprintf(stdout, "matrix_to_linked_list:\n");
    test_matrix_to_linked_list();

    fprintf(stdout, "remove_node\n");
    test_remove_node();

    fprintf(stdout, "unremove_node\n");
    test_unremove_node();

    fprintf(stdout, "cover_column:\n");
    test_cover_column();

    fprintf(stdout, "uncover_column_1:\n");
    test_uncover_column_1();

    fprintf(stdout, "uncover_column_2:\n");
    test_uncover_column_2();

    fprintf(stdout, "get_shortest_column:\n");
    test_get_shortest_column();

    fprintf(stdout, "solve_exact_cover_1:\n");
    test_solve_exact_cover_1((SetCover){.items=(int[]){0,2}, .count=2});

    fprintf(stdout, "solve_exact_cover_2:\n");
    test_solve_exact_cover_2((SetCover){.items=(int[]){1,3,5}, .count=3});
    
    fprintf(stdout, "solve_exact_cover_3:\n");
    test_solve_exact_cover_3((SetCover){.items=(int[]){2,3,4}, .count=3});

    fprintf(stdout, "solve_exact_cover_incomplete:\n");
    test_solve_exact_cover_incomplete((SetCover){.items=(int[]){}, .count=0});

    fprintf(stdout, "solve_sudoku_1:\n");
    test_solve_sudoku_1();

    fprintf(stdout, "solve_sudoku_2:\n");
    test_solve_sudoku_2();

    fprintf(stdout, "count_solutions:\n");
    test_count_solutions();

    // test_solvers();
    
    // printf("generating random sudoku with unique solution..\n");
    // generate_and_solve_random_sudoku();

    return 0;
}