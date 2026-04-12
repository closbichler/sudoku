#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define EXACT_COVER_IMPLEMENTATION
#include "exact_cover.h"

// -- Helper functions --

uint8_t** create_empty_constraint_sets(int num_sets, int set_size)
{
    uint8_t **constraint_sets = malloc(num_sets * sizeof(uint8_t*));
    for (int i = 0; i < num_sets; i++) {
        constraint_sets[i] = malloc(set_size * sizeof(uint8_t));
        memset(constraint_sets[i], 0, set_size * sizeof(uint8_t));
    }
    return constraint_sets;
}

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

void exact_print_column(DLXNode *head) 
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

void exact_print_columns(DLXColumn *root)
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
        exact_print_column(c->head);
        c = c->next;
    } while (c != root);
}

// -- Unit tests --

bool test_exact_constraints_to_dlx()
{
    uint8_t **constraint_sets = create_empty_constraint_sets(3, 3);
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
    uint8_t **constraint_sets = create_empty_constraint_sets(3, 3);
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
    uint8_t **constraint_sets = create_empty_constraint_sets(3, 3);
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
    uint8_t **constraint_sets = create_empty_constraint_sets(3, 3);
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
    uint8_t **constraint_sets = create_empty_constraint_sets(3, 3);
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
    uint8_t **constraint_sets = create_empty_constraint_sets(6, 7);

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
    uint8_t **constraint_sets = create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,2 };
    constraint_sets[1] = (uint8_t[]) { 0,3,0 };
    constraint_sets[2] = (uint8_t[]) { 4,5,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);
    DLXColumn *c = exact_get_shortest_column(root);
    if (c->id != 2) return false;
    if (c != root->next->next->next) return false;

    constraint_sets = create_empty_constraint_sets(6, 7);
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
    uint8_t **constraint_sets = create_empty_constraint_sets(3, 3);
    constraint_sets[0] = (uint8_t[]) { 1,0,1 };
    constraint_sets[1] = (uint8_t[]) { 1,1,0 };
    constraint_sets[2] = (uint8_t[]) { 0,1,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 3, 3);
    SetCover cover = {0};
    
    exact_create_setcover_hashtable(16);
    int result = exact_solve(root, &cover, true, -1);
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_2(SetCover expected_cover)
{
    uint8_t **constraint_sets = create_empty_constraint_sets(6, 7);
    constraint_sets[0] = (uint8_t[]) { 1,0,0,1,0,0,1 };
    constraint_sets[1] = (uint8_t[]) { 1,0,0,1,0,0,0 };
    constraint_sets[2] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[3] = (uint8_t[]) { 0,0,1,0,1,1,0 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,1 };
    constraint_sets[5] = (uint8_t[]) { 0,1,0,0,0,0,1 };
        DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    SetCover cover = {0};

    exact_create_setcover_hashtable(16);
    int result = exact_solve(root, &cover, true, -1);
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_3(SetCover expected_cover)
{
    uint8_t **constraint_sets = create_empty_constraint_sets(6, 7);
    constraint_sets[0] = (uint8_t[]) { 0,1,0,0,1,0,0 };
    constraint_sets[1] = (uint8_t[]) { 0,0,1,0,0,1,0 };
    constraint_sets[2] = (uint8_t[]) { 1,0,0,0,0,0,0 };
    constraint_sets[3] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,0 };
    constraint_sets[5] = (uint8_t[]) { 0,0,0,0,0,0,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    SetCover cover = {0};

    int result = exact_solve(root, &cover, true, -1);
    if (result != 1) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

bool test_solve_exact_cover_incomplete(SetCover expected_cover)
{
    uint8_t **constraint_sets = create_empty_constraint_sets(6, 7);
    constraint_sets[0] = (uint8_t[]) { 0,1,0,0,1,0,0 };
    constraint_sets[1] = (uint8_t[]) { 0,0,1,0,0,1,0 };
    constraint_sets[2] = (uint8_t[]) { 1,0,0,1,0,0,0 };
    constraint_sets[3] = (uint8_t[]) { 0,0,0,1,1,0,1 };
    constraint_sets[4] = (uint8_t[]) { 0,1,1,0,0,1,0 };
    constraint_sets[5] = (uint8_t[]) { 0,0,0,0,0,0,0 };
    DLXColumn *root = exact_constraints_to_dlx(constraint_sets, 6, 7);
    SetCover cover = {0};

    exact_create_setcover_hashtable(16);
    int result = exact_solve(root, &cover, true, -1);
    if (result != 0) return false;
    if (!set_covers_equal(expected_cover, cover)) return false;
    return true;
}

int main() 
{
    bool test_unit = true;

    fprintf(stdout, "\n\x1b[36m\x1b[1m========================================\n");
    fprintf(stdout, "  \x1b[33mExact Cover — Unit Tests\x1b[36m\n");
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

    return 0;
}