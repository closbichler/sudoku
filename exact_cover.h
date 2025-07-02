#ifndef EXACT_COVER
#define EXACT_COVER

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

bool exact_cover(Matrix *sets, Cover *cover);
void exact_cover_example();
void exact_cover_print_sets(Matrix sets, Cover cover);
bool exact_cover_delete_set_by_id(Matrix *sets, int id);
bool exact_cover_delete_possibility_from_sets(Matrix* sets, int set_id);
void exact_cover_clone_matrix(Matrix* sets, Matrix to_clone);

int    exact_cover_solve_sudoku(Sudoku* s);
Matrix exact_cover_create_sudoku_constraint_sets(int size, int block_size);

#endif // EXACT_COVER

#ifdef EXACT_COVER_IMPLEMENTATION

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

bool exact_cover(Matrix *sets, Cover *cover)
{
    if (sets->count == 0)
        return true;

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

    if (ones == 0) 
        return false;

    for (int row = 0; row < sets->items[row].count; row++) {
        if (sets->items[row].items[column] == 1) {
            // save context
            int set_id = sets->items[row].id;
            da_append(cover, sets->items[row].id);
            Matrix old_sets = {0};
            exact_cover_clone_matrix(&old_sets, *sets);

            exact_cover_delete_possibility_from_sets(sets, set_id);

            if (exact_cover(sets, cover))
                return true;

            // restore context
            exact_cover_clone_matrix(sets, old_sets);
            da_remove(cover, cover->count-1);
        }
    }

    return false;
}

int exact_cover_solve_sudoku(Sudoku* s)
{
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
        
    if (!exact_cover(&sets_covered, &cover)) return 0;
        
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

    return 1;
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

    Matrix initial_sets = {0};
    exact_cover_clone_matrix(&initial_sets, sets);
    Cover cover = {0};
    if (!exact_cover(&sets, &cover)) printf("no solution\n");
    exact_cover_print_sets(initial_sets, cover);
}

#endif // EXACT_COVER_IMPLEMENTATION