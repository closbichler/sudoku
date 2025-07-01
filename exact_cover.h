#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#ifndef N
#define N 9
#endif
#ifndef C
#define C 3
#endif
#include "sudoku.h"
#define DA_INIT_CAP 256

#define da_reserve(da, expected_capacity)                                              \
    do                                                                                 \
    {                                                                                  \
        if ((expected_capacity) > (da)->capacity) {                                    \
            if ((da)->capacity == 0) {                                                 \
                (da)->capacity = DA_INIT_CAP;                                          \
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

void delete_set_by_id(Matrix *sets, int id)
{
    for (int i = 0; i < sets->count; i++) {
        if (sets->items[i].id == id) {
            da_remove(sets, i);
            return;
        }
    }
}

void print_sets(Matrix sets, Cover cover)
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
                printf("\033[31m%d", val);
            else 
                printf("\033[0m%d", val);
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
        printf("\033[0m\n");
    }

    printf("sum: ");
    for (int i = 0; i < sets.items[0].count; i++) {
        printf("%d", sum[i]);
    }
    printf("\n");
}

void clone_matrix(Matrix* sets, Matrix to_clone)
{
    *sets = (Matrix) {0};
    for (int i = 0; i < to_clone.count; i++) {
        Set set = (Set){0};
        set.id = i;
        da_append(sets, set);
        for (int j = 0; j < to_clone.items[i].count; j++) {
            da_append(&sets->items[i], to_clone.items[i].items[j]);
        }
    }
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
            da_append(cover, sets->items[row].id);
            Matrix old_sets = {0};
            clone_matrix(&old_sets, *sets);

            Cover rows_to_delete = {0};
            Cover cols_to_delete = {0};
            for (int i = 0; i < sets->items[row].count; i++) {
                if (sets->items[row].items[i] == 1) {
                    for (int j = 0; j < sets->count; j++) {
                        if (sets->items[j].items[i] == 1) {
                            da_append(&rows_to_delete, sets->items[j].id);
                        }
                    }
                    da_append(&cols_to_delete, i);
                }
            }

            for (int i = 0; i < rows_to_delete.count; i++) {
                delete_set_by_id(sets, rows_to_delete.items[i]);
            }
            
            for (int i = cols_to_delete.count - 1; i >= 0; i--) {
                for (int j = 0; j < sets->count; j++) {
                    da_remove(&(sets->items[j]), cols_to_delete.items[i]);
                }
            }

            if (exact_cover(sets, cover))
                return true;

            // restore context
            clone_matrix(sets, old_sets);
            da_remove(cover, cover->count-1);
        }
    }

    return false;
}

void small_exact_cover()
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
    clone_matrix(&initial_sets, sets);
    Cover cover = {0};
    if (!exact_cover(&sets, &cover)) printf("no solution\n");
    print_sets(initial_sets, cover);
}

void solve_sudoku(char** sudoku)
{
    Matrix sets = create_constraint_sets();

    for (int i=0; i<sudok)

    Cover cover = {0};
    
    if (!exact_cover(&sets, &cover))
        printf("no solution\n");
    
    char **sudoku = create_empty_sudoku();
    for (int i = 0; i < sets.count; i++) {
        bool selected = false;
        for (int k = 0; k < cover.count; k++) {
            if (sets.items[i].id == cover.items[k]) {
                selected = true;
                break;
            }
        }

        if (!selected)
            continue;

        for (int j = 0; j < sets.items[i].count; j++) {
            if (sets.items[i].items[j] == 1) {
                int x = i / (N * N);
                int y = (i % (N * N)) / N;
                int val = i % N + 1;
                sudoku[x][y] = val;
            }
        }
    }

    // print_sets(sets, cover);
    print_sudoku(sudoku);
}

Matrix create_constraint_sets()
{
    Matrix sets = {0};
    for (int i = 0; i < N * N * N; i++) {
        Set set = (Set){0};
        set.id = i;
        da_append(&sets, set);
    }
    
    // cell constraints
    for (int i=0; i<N*N && true; i++) {
        for (int j=0; j<N*N; j++) {
            for (int k=0; k<N; k++) {
                if (i == j)
                    da_append(&sets.items[j*N + k], 1);
                else
                    da_append(&sets.items[j*N + k], 0);
            }
        }
    }

    // row constraints
    for (int i = 0; i < N * N && true; i++) {
        for (int j = 0; j < N * N; j++) {
            for (int k = 0; k < N; k++) {
                if (j == k + i - i%N)
                    da_append(&sets.items[i*N + k], 1);
                else
                    da_append(&sets.items[i*N + k], 0);
            }
        }
    }

    // column constraints
    for (int i = 0; i < N * N && true; i++) {
        for (int j = 0; j < N * N; j++) {
            for (int k = 0; k < N; k++) {
                if (j == (i*N + k) % (N*N))
                    da_append(&sets.items[i*N + k], 1);
                else
                    da_append(&sets.items[i*N + k], 0);
            }
        }
    } 

    // block constraints
    int col_offset = sets.items[0].count;
    for (int i=0; i<N*N*N; i++) {
        for (int j=0; j<N*N; j++) {
            da_append(&sets.items[i], 0);
        }
    }
    
    for (int bx=0; bx<N/C; bx++) {
        for (int by=0; by<N/C; by++) {
            for (int n=0; n<N; n++) {
                for (int i=0; i<C; i++) {
                    for (int j=0; j<C; j++) {
                        int cell_x = bx * C + i;
                        int cell_y = by * C + j;
                        int row = cell_x * N * N + cell_y * N + n;
                        int col = col_offset + n + bx * N * C + by * N;

                        sets.items[row].items[col] = 1;
                    }
                }
            }
        }
    }

    return sets;
}