#ifndef EXACT_COVER
#define EXACT_COVER

#include <stdbool.h>
#include <stdlib.h>

#include "sudoku.h"

/*
    Naive implementation of exact cover
*/

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

bool exact_cover_is_solvable(Matrix *sets, Cover *cover);
int  exact_cover_solutions(Matrix *sets, Cover *cover);
void exact_cover_print_sets(Matrix sets, Cover cover);
bool exact_cover_delete_set_by_id(Matrix *sets, int id);
bool exact_cover_delete_possibility_from_sets(Matrix *sets, int set_id);
void exact_cover_clone_matrix(Matrix *sets, Matrix to_clone);

bool   exact_cover_solve_sudoku(Sudoku *s);
int    exact_cover_sudoku_solutions(Sudoku s);
Matrix exact_cover_create_sudoku_constraint_sets(int size, int block_size);

/*
    DLX implementation of exact cover
*/

typedef struct Node {
   struct Node *left, *right;
   struct Node *up, *down;
   struct Column *c;
   char val;
   int  set_id;
} Node;

typedef struct Column {
    struct Column *prev, *next;
    Node *head;
    int len;
    int id;
} Column;

Column* dlx_matrix_to_linked_list(Matrix matrix);
void    dlx_print_columns(Column *root);
bool    dlx_solve_exact_cover(Column *root, Cover *cover);
void    dlx_delete_possibility(Column *root, int set_id);

bool   dlx_solve_sudoku(Sudoku *s);
int    dlx_sudoku_solutions(Sudoku s);

#endif // EXACT_COVER

#ifdef EXACT_COVER_IMPLEMENTATION
#ifndef EXACT_COVER_IMPLEMENTED
#define EXACT_COVER_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#include "sudoku.h"

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

bool exact_cover_solve_sudoku(Sudoku* s)
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

int exact_cover_sudoku_solutions(Sudoku s)
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
    Column *root = dlx_matrix_to_linked_list(sets);
    if (!dlx_solve_exact_cover(root, &cover)) printf("no solution\n");
    exact_cover_print_sets(sets, cover);
}


void dlx_print_columns(Column *root)
{
    Column *c = root->next;
    int i = 0;
    printf("Col (len): \n");
    do {
        printf("%03d (%02d): ", c->id, c->len);
        if (c->head == NULL) {
            c = c->next;
            printf("\n");
            continue;
        }
        Node *n = c->head;
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

Column* dlx_matrix_to_linked_list(Matrix matrix)
{
    Column *root = malloc(sizeof(Column));
    root->next = root;
    root->prev = root;

    Column *c = root;
    for (int i=0; i<matrix.items[0].count; i++) {
        Column *new_col = malloc(sizeof(Column));
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
        Column *c = root;
        Node *last_node = NULL;
        Node *first_node = NULL;
        for (int j=0; j<matrix.items[i].count; j++) {            
            c = c->next;
            int val = matrix.items[i].items[j];
            if (val == 0) continue;

            Node *new_node = malloc(sizeof(Node));
            new_node->val = val;
            new_node->set_id = matrix.items[i].id;
            new_node->c = c;
            if (last_node == NULL) {
                last_node = new_node;
                first_node = new_node;
            }
            last_node->right = new_node;
            new_node->left = last_node;
            last_node = new_node;
            
            if (c->head == NULL) {
                c->head = new_node;
                c->head->up = c->head;
                c->head->down = c->head;
            } else {
                new_node->down = c->head;
                new_node->up = c->head->up;
                c->head->up->down = new_node;
                c->head->up = new_node;
            }
            c->len++;
        }
        if (last_node != NULL && first_node != NULL) {
            last_node->right = first_node;
            first_node->left = last_node;
        }
    }

    return root;
}

void dlx_remove_element_from_col(Node *n)
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

void dlx_unremove_element_from_col(Node *n)
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

void dlx_cover_column(Column *c)
{
    c->prev->next = c->next;
    c->next->prev = c->prev;
    Node *n = c->head;
    if (n == NULL) return;
    do {
        Node *neighbor = n->right;
        while (neighbor != n) {
            dlx_remove_element_from_col(neighbor);
            neighbor = neighbor->right;
        }
        n = n->down;
    } while (n != c->head);
}

void dlx_uncover_column(Column *c) 
{
    c->prev->next = c;
    c->next->prev = c;
    Node *n = c->head;
    if (n == NULL) return;
    do {
        Node *neighbor = n->right;
        while (neighbor != n) {
            dlx_unremove_element_from_col(neighbor);
            neighbor = neighbor->right;
        }
        n = n->up;
    } while (n != c->head);
}

bool dlx_solve_exact_cover(Column *root, Cover *cover)
{
    if (root->next == root) return true;

    // choose column with least 1s
    Column *c_iter = root->next;
    Column *c = c_iter;
    int min_len = c->len;
    while (c_iter != root) {
        if (c_iter->len < min_len) {
            c = c_iter;
            min_len = c->len;
        }
        c_iter = c_iter->next;
    }
    
    if (min_len <= 0) return false;
    
    Node *n = c->head;
    do {    
        // cover row
        da_append(cover, n->set_id);
        Node *neighbor = n;
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

int dlx_exact_cover_solutions(Column *root, Cover *cover)
{
    if (root->next == root) return 1;

    // choose column with least 1s
    Column *c_iter = root->next;
    Column *c = c_iter;
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
    Node *n = c->head;
    if (n == NULL) {
        // TODO: there's some bug in the covering and uncovering functions
        printf("no head, but col len = %d\n", min_len);
        return 0;
    }
    do {   
        // cover row
        da_append(cover, n->set_id);
        Node *neighbor = n;
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

void dlx_delete_possibility(Column *root, int set_id)
{
    Column *c = root->next;
    while (c != root) {
        Node *n = c->head;
        do {
            if (n->set_id == set_id) {
                Node *neighbor = n;
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

bool dlx_solve_sudoku(Sudoku *s)
{
    if (!sudoku_is_valid(*s)) return false;

    Matrix sets = exact_cover_create_sudoku_constraint_sets(9, 3);    
    Column *root = dlx_matrix_to_linked_list(sets);

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

int dlx_sudoku_solutions(Sudoku s)
{
    if (!sudoku_is_valid(s)) return 0;

    Matrix sets = exact_cover_create_sudoku_constraint_sets(9, 3);    
    Column *root = dlx_matrix_to_linked_list(sets);

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

#endif // EXACT_COVER_IMPLEMENTED
#endif // EXACT_COVER_IMPLEMENTATION