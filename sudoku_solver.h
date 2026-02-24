#ifndef SUDOKU_SOLVER
#define SUDOKU_SOLVER

#include <stdbool.h>
#include <stdlib.h>

#include "sudoku.h"

bool   solver_fill_sudoku(Sudoku *s);
int    solver_count_solutions(Sudoku s);

bool   solver_exact_cover_fill_sudoku(Sudoku *s);
int    solver_exact_cover_count_solutions(Sudoku s);

#endif // SUDOKU_SOLVER

#ifdef SUDOKU_SOLVER_IMPLEMENTATION
#ifndef SUDOKU_SOLVER_IMPLEMENTED
#define SUDOKU_SOLVER_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#include "sudoku.h"

typedef struct {
    int id;
    int *items;
    size_t count;
    size_t capacity;
} Set;

typedef struct {
    Set *items;
    size_t count;
    size_t capacity;
} Matrix;

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Cover;

typedef struct DLXNode {
   struct DLXNode *left, *right;
   struct DLXNode *up, *down;
   struct DLXColumn *c;
   char val;
   int  set_id;
} DLXNode;

typedef struct DLXColumn {
    struct DLXColumn *prev, *next;
    DLXNode *head;
    int len;
    int id;
} DLXColumn;

// DA Macros from www.github.com/tsoding/nob.h

#define da_reserve(da, expected_capacity)                                              \
    do                                                                                 \
    {                                                                                  \
        if ((expected_capacity) > (da)->capacity) {                                    \
            if ((da)->capacity == 0) {                                                 \
                (da)->capacity = 256;                                          \
            }                                                                          \
            while ((expected_capacity) > (da)->capacity) {                             \
                (da)->capacity *= 2;                                                   \
            }                                                                          \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                         \
        }                                                                              \
    } while (0)

#define da_append(da, item)                  \
    do                                       \
    {                                        \
        da_reserve((da), (da)->count + 1);   \
        (da)->items[(da)->count++] = (item); \
    } while (0)

#define da_remove(da, i)                              \
    do                                                \
    {                                                 \
        size_t jj = (i);                              \
        assert(jj < (da)->count);                     \
        (da)->count--;                                \
        for (int jjj=jj; jjj<(da)->count; jjj++) {    \
            (da)->items[jjj] = (da)->items[jjj+1];    \
        }                                             \
    } while (0)

bool exact_cover_delete_set_by_id(Matrix *sets, int id)
{
    for (int i = 0; i < sets->count; i++) {
        if (sets->items[i].id == id) {
            da_remove(sets, i);
            return true;
        }
    }
    return false;
}

void exact_cover_print_sets(Matrix sets, Cover cover)
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

void exact_cover_clone_matrix(Matrix* sets, Matrix to_clone)
{
    *sets = (Matrix) {0};
    for (int i = 0; i < to_clone.count; i++) {
        Set set = (Set){0};
        set.id = to_clone.items[i].id;
        da_append(sets, set);
        for (int j = 0; j < to_clone.items[i].count; j++) {
            da_append(&sets->items[i], to_clone.items[i].items[j]);
        }
    }
}

bool exact_cover_delete_possibility_from_sets(Matrix* sets, int set_id)
{
    int set = -1;
    for (int k=0; k<sets->count; k++) {
        if (sets->items[k].id == set_id) {
            set = k;
            break;
        }
    }
    if (set == -1) return false;

    Cover rows_to_delete = {0};
    Cover cols_to_delete = {0};
    for (int i = 0; i < sets->items[set].count; i++) {
        if (sets->items[set].items[i] == 1) {
            for (int j = 0; j < sets->count; j++) {
                if (sets->items[j].items[i] == 1) {
                    da_append(&rows_to_delete, sets->items[j].id);
                }
            }
            da_append(&cols_to_delete, i);
        }
    }

    for (int i = 0; i < rows_to_delete.count; i++) {
        exact_cover_delete_set_by_id(sets, rows_to_delete.items[i]);
    }
    
    for (int i = cols_to_delete.count - 1; i >= 0; i--) {
        for (int j = 0; j < sets->count; j++) {
            da_remove(&(sets->items[j]), cols_to_delete.items[i]);
        }
    }
    return true;
}

Matrix exact_cover_create_sudoku_constraint_sets(int size, int block_size)
{
    Matrix sets = {0};
    for (int i = 0; i < size * size * size; i++) {
        Set set = (Set){0};
        set.id = i;
        da_append(&sets, set);
    }
    
    // cell constraints
    for (int i=0; i<size*size && true; i++) {
        for (int j=0; j<size*size; j++) {
            for (int k=0; k<size; k++) {
                if (i == j)
                    da_append(&sets.items[j*size + k], 1);
                else
                    da_append(&sets.items[j*size + k], 0);
            }
        }
    }

    // row constraints
    for (int i = 0; i < size * size && true; i++) {
        for (int j = 0; j < size * size; j++) {
            for (int k = 0; k < size; k++) {
                if (j == k + i - i%size)
                    da_append(&sets.items[i*size + k], 1);
                else
                    da_append(&sets.items[i*size + k], 0);
            }
        }
    }

    // column constraints
    for (int i = 0; i < size * size && true; i++) {
        for (int j = 0; j < size * size; j++) {
            for (int k = 0; k < size; k++) {
                if (j == (i*size + k) % (size*size))
                    da_append(&sets.items[i*size + k], 1);
                else
                    da_append(&sets.items[i*size + k], 0);
            }
        }
    } 

    // block constraints
    int col_offset = sets.items[0].count;
    for (int i=0; i<size*size*size; i++) {
        for (int j=0; j<size*size; j++) {
            da_append(&sets.items[i], 0);
        }
    }
    
    for (int bx=0; bx<size/block_size; bx++) {
        for (int by=0; by<size/block_size; by++) {
            for (int n=0; n<size; n++) {
                for (int i=0; i<block_size; i++) {
                    for (int j=0; j<block_size; j++) {
                        int cell_x = bx * block_size + i;
                        int cell_y = by * block_size + j;
                        int row = cell_x * size * size + cell_y * size + n;
                        int col = col_offset + n + bx * size * block_size + by * size;

                        sets.items[row].items[col] = 1;
                    }
                }
            }
        }
    }

    return sets;
}

bool exact_cover_is_solvable(Matrix *sets, Cover *cover)
{
    if (sets->count == 0) return true;

    // choose column with least 1s
    int column = 0, ones = INT_MAX;
    for (int i=0, j=0; i<sets->items[0].count; i++) {
        int current_ones = 0;
        for (j=0; j<sets->count; j++) {
            current_ones += sets->items[j].items[i];
        }
        if (current_ones < ones) {
            column = i;
            ones = current_ones;
        }
    }

    if (ones == 0) return false;

    for (int row = 0; row < sets->items[row].count; row++) {
        if (sets->items[row].items[column] == 1) {
            // save context
            int set_id = sets->items[row].id;
            da_append(cover, sets->items[row].id);
            Matrix old_sets = {0};
            exact_cover_clone_matrix(&old_sets, *sets);

            exact_cover_delete_possibility_from_sets(sets, set_id);

            if (exact_cover_is_solvable(sets, cover))
                return true;

            // restore context
            exact_cover_clone_matrix(sets, old_sets);
            da_remove(cover, cover->count-1);
        }
    }

    return false;
}

int exact_cover_solutions(Matrix *sets, Cover *cover)
{
    if (sets->count == 0) return 1;
    
    // choose column with least 1s
    int column = 0, ones = INT_MAX;
    for (int i=0, j=0; i<sets->items[0].count; i++) {
        int current_ones = 0;
        for (j=0; j<sets->count; j++) {
            current_ones += sets->items[j].items[i];
        }
        if (current_ones < ones) {
            column = i;
            ones = current_ones;
        }
    }
    
    if (ones == 0) return 0;
    
    int solutions = 0;
    for (int row = 0; row < sets->items[row].count; row++) {
        if (sets->items[row].items[column] == 1) {
            // save context
            int set_id = sets->items[row].id;
            da_append(cover, sets->items[row].id);
            Matrix old_sets = {0};
            exact_cover_clone_matrix(&old_sets, *sets);

            exact_cover_delete_possibility_from_sets(sets, set_id);
            solutions += exact_cover_solutions(sets, cover);

            // restore context
            exact_cover_clone_matrix(sets, old_sets);
            da_remove(cover, cover->count-1);
        }
    }

    return solutions;
}

bool solver_exact_cover_fill_sudoku(Sudoku* s)
{
    if (!sudoku_is_valid(*s)) return false;
    Matrix sets = exact_cover_create_sudoku_constraint_sets(s->size, s->block_size);

    for (int i=0; i<s->size; i++) {
        for (int j=0; j<s->size; j++) {
            int val = s->field[i][j];
            if (val == 0) continue;
            int set_id = i * s->size * s->size + j * s->size + val - 1;
            exact_cover_delete_possibility_from_sets(&sets, set_id);
        }
    }
   
    Matrix sets_covered = {0};
    exact_cover_clone_matrix(&sets_covered, sets);
    Cover cover = {0};
        
    if (!exact_cover_is_solvable(&sets_covered, &cover)) 
        return false;
        
    for (int i = 0; i < sets.count; i++) {
        bool selected = false;
        for (int k = 0; k < cover.count; k++) {
            if (sets.items[i].id == cover.items[k]) {
                selected = true;
                break;
            }
        }

        if (!selected) continue;

        for (int j = 0; j < sets.items[i].count; j++) {
            if (sets.items[i].items[j] == 1) {
                int x = sets.items[i].id / (s->size * s->size);
                int y = (sets.items[i].id % (s->size * s->size)) / s->size;
                int val = sets.items[i].id % s->size + 1;
                s->field[x][y] = val;
            }
        }
    }

    return true;
}

int solver_exact_cover_count_solutions(Sudoku s)
{
    if (!sudoku_is_valid(s)) return 0;
    Matrix sets = exact_cover_create_sudoku_constraint_sets(s.size, s.block_size);

    for (int i=0; i<s.size; i++) {
        for (int j=0; j<s.size; j++) {
            int val = s.field[i][j];
            if (val == 0) continue;
            int set_id = i * s.size * s.size + j * s.size + val - 1;
            exact_cover_delete_possibility_from_sets(&sets, set_id);
        }
    }
   
    Matrix sets_covered = {0};
    exact_cover_clone_matrix(&sets_covered, sets);
    Cover cover = {0};
        
    return exact_cover_solutions(&sets_covered, &cover);
}


void dlx_print_columns(DLXColumn *root)
{
    DLXColumn *c = root->next;
    int i = 0;
    printf("Col (len): \n");
    do {
        printf("%03d (%02d): ", c->id, c->len);
        if (c->head == NULL) {
            c = c->next;
            printf("\n");
            continue;
        }
        DLXNode *n = c->head;
        do {
            if (n == c->head) printf(":head:=");
            printf("%d->", n->val);
            if (n->down == c->head) printf(":down-head:");
            if (n == c->head->up) printf(":head-up:");
            n = n->down;
            i++;
        } while (n != c->head);
        printf("\n");
        c = c->next;
    } while (c != root);
}

DLXColumn* dlx_matrix_to_linked_list(Matrix matrix)
{
    DLXColumn *root = malloc(sizeof(DLXColumn));
    root->next = root;
    root->prev = root;

    DLXColumn *c = root;
    for (int i=0; i<matrix.items[0].count; i++) {
        DLXColumn *new_col = malloc(sizeof(DLXColumn));
        new_col->len = 0;
        new_col->id = i;
        new_col->head = NULL;
        new_col->prev = c;
        new_col->next = root;
        root->prev = new_col;
        c->next = new_col;
        c = new_col;
    }

    
    for (int i=0; i<matrix.count; i++) {
        DLXColumn *c = root;
        DLXNode *last_DLXnode = NULL;
        DLXNode *first_DLXnode = NULL;
        for (int j=0; j<matrix.items[i].count; j++) {            
            c = c->next;
            int val = matrix.items[i].items[j];
            if (val == 0) continue;

            DLXNode *new_DLXnode = malloc(sizeof(DLXNode));
            new_DLXnode->val = val;
            new_DLXnode->set_id = matrix.items[i].id;
            new_DLXnode->c = c;
            if (last_DLXnode == NULL) {
                last_DLXnode = new_DLXnode;
                first_DLXnode = new_DLXnode;
            }
            last_DLXnode->right = new_DLXnode;
            new_DLXnode->left = last_DLXnode;
            last_DLXnode = new_DLXnode;
            
            if (c->head == NULL) {
                c->head = new_DLXnode;
                c->head->up = c->head;
                c->head->down = c->head;
            } else {
                new_DLXnode->down = c->head;
                new_DLXnode->up = c->head->up;
                c->head->up->down = new_DLXnode;
                c->head->up = new_DLXnode;
            }
            c->len++;
        }
        if (last_DLXnode != NULL && first_DLXnode != NULL) {
            last_DLXnode->right = first_DLXnode;
            first_DLXnode->left = last_DLXnode;
        }
    }

    return root;
}

void dlx_remove_element_from_col(DLXNode *n)
{
    if (n->down == n) {
        n->c->head = NULL;
    } else {
        if (n == n->c->head) {
            n->c->head = n->down;
        }
    }
    n->up->down = n->down;
    n->down->up = n->up;
    n->c->len--;
}

void dlx_unremove_element_from_col(DLXNode *n)
{
    if (n->c->head == NULL) {
        n->c->head = n;
        n->c->len++;
    } else if (n->up->down == n && n->down->up == n) {
        return;
    } 
    n->up->down = n;
    n->down->up = n;
    n->c->len++;
}

void dlx_cover_column(DLXColumn *c)
{
    c->prev->next = c->next;
    c->next->prev = c->prev;
    DLXNode *n = c->head;
    if (n == NULL) return;
    do {
        DLXNode *neighbor = n->right;
        while (neighbor != n) {
            dlx_remove_element_from_col(neighbor);
            neighbor = neighbor->right;
        }
        n = n->down;
    } while (n != c->head);
}

void dlx_uncover_column(DLXColumn *c) 
{
    c->prev->next = c;
    c->next->prev = c;
    DLXNode *n = c->head;
    if (n == NULL) return;
    do {
        DLXNode *neighbor = n->right;
        while (neighbor != n) {
            dlx_unremove_element_from_col(neighbor);
            neighbor = neighbor->right;
        }
        n = n->up;
    } while (n != c->head);
}

bool dlx_solve_exact_cover(DLXColumn *root, Cover *cover)
{
    if (root->next == root) return true;

    // choose column with least 1s
    DLXColumn *c_iter = root->next;
    DLXColumn *c = c_iter;
    int min_len = c->len;
    while (c_iter != root) {
        if (c_iter->len < min_len) {
            c = c_iter;
            min_len = c->len;
        }
        c_iter = c_iter->next;
    }
    
    if (min_len <= 0) return false;
    
    DLXNode *n = c->head;
    do {    
        // cover row
        da_append(cover, n->set_id);
        DLXNode *neighbor = n;
        do {
            dlx_cover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        if (dlx_solve_exact_cover(root, cover))
            return true;

        // uncover row
        da_remove(cover, cover->count-1);
        neighbor = n;
        do {
            dlx_uncover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        n = n->down;
    } while (n != c->head);

    return false;
}

int dlx_exact_cover_solutions(DLXColumn *root, Cover *cover)
{
    if (root->next == root) return 1;

    // choose column with least 1s
    DLXColumn *c_iter = root->next;
    DLXColumn *c = c_iter;
    int min_len = c->len;
    while (c_iter != root) {
        if (c_iter->len < min_len) {
            c = c_iter;
            min_len = c->len;
        }
        c_iter = c_iter->next;
    }
    
    if (min_len <= 0) return 0;
    
    int solutions = 0;
    DLXNode *n = c->head;
    if (n == NULL) {
        // TODO: there's some bug in the covering and uncovering functions
        printf("no head, but col len = %d\n", min_len);
        return 0;
    }
    do {   
        // cover row
        da_append(cover, n->set_id);    
        DLXNode *neighbor = n;
        do {
            dlx_cover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        solutions += dlx_exact_cover_solutions(root, cover);

        // uncover row
        da_remove(cover, cover->count-1);
        neighbor = n;
        do {
            dlx_uncover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        n = n->down;
    } while (n != c->head);

    return solutions;
}

void dlx_delete_possibility(DLXColumn *root, int set_id)
{
    DLXColumn *c = root->next;
    while (c != root) {
        DLXNode *n = c->head;
        do {
            if (n->set_id == set_id) {
                DLXNode *neighbor = n;
                do {
                    dlx_cover_column(neighbor->c);
                    neighbor = neighbor->right;
                } while (n != neighbor);

                return;
            }
            n = n->down;
        } while (n != c->head);
        c = c->next;
    }
}

bool solver_solve_sudoku(Sudoku *s)
{
    if (!sudoku_is_valid(*s)) return false;

    Matrix sets = exact_cover_create_sudoku_constraint_sets(9, 3);    
    DLXColumn *root = dlx_matrix_to_linked_list(sets);

    for (int i=0; i<s->size; i++) {
        for (int j=0; j<s->size; j++) {
            int val = s->field[i][j];
            if (val == 0) continue;
            int set_id = i * s->size * s->size + j * s->size + val - 1;
            dlx_delete_possibility(root, set_id);
        }
    }

    Cover cover = {0};
    if (!dlx_solve_exact_cover(root, &cover)) return false;

    for (int k = 0; k < cover.count; k++) {
        int id = cover.items[k];

        int x = id / (s->size * s->size);
        int y = (id % (s->size * s->size)) / s->size;
        int val = id % s->size + 1;
        s->field[x][y] = val;
    }
    return true;
}

int solver_count_solutions(Sudoku s)
{
    if (!sudoku_is_valid(s)) return 0;

    Matrix sets = exact_cover_create_sudoku_constraint_sets(9, 3);    
    DLXColumn *root = dlx_matrix_to_linked_list(sets);

    for (int i=0; i<s.size; i++) {
        for (int j=0; j<s.size; j++) {
            int val = s.field[i][j];
            if (val == 0) continue;
            int set_id = i * s.size * s.size + j * s.size + val - 1;
            dlx_delete_possibility(root, set_id);
        }
    }

    Cover cover = {0};
    return dlx_exact_cover_solutions(root, &cover);
}

void exact_cover_example()
{
    Matrix sets = {0};
    for (int i = 0; i < 6; i++) {
        Set set = (Set){0};
        set.id = i;
        set.count = 7;
        set.capacity = 7;
        da_append(&sets, set);
    }

    sets.items[0].items = (int[]) { 1,0,0,1,0,0,1 };
    sets.items[1].items = (int[]) { 1,0,0,1,0,0,0 };
    sets.items[2].items = (int[]) { 0,0,0,1,1,0,1 };
    sets.items[3].items = (int[]) { 0,0,1,0,1,1,0 };
    sets.items[4].items = (int[]) { 0,1,1,0,0,1,1 };
    sets.items[5].items = (int[]) { 0,1,0,0,0,0,1 };

    // sets.items[0].items = (int[]) { 0,1,0,0,1,0,0 };
    // sets.items[1].items = (int[]) { 0,0,1,0,0,1,0 };
    // sets.items[2].items = (int[]) { 1,0,0,0,0,0,0 };
    // sets.items[3].items = (int[]) { 0,0,0,1,1,0,1 };
    // sets.items[4].items = (int[]) { 0,1,1,0,0,1,0 };
    // sets.items[5].items = (int[]) { 0,0,0,0,0,0,0 };

    
    Cover cover = {0};
    DLXColumn *root = dlx_matrix_to_linked_list(sets);
    if (!dlx_solve_exact_cover(root, &cover)) printf("no solution\n");
    exact_cover_print_sets(sets, cover);
}   

#endif // SUDOKU_SOLVER_IMPLEMENTED
#endif // SUDOKU_SOLVER_IMPLEMENTATION