/*----------------------
 | SUS - Sudoku Solver in C
 |
 | Author: Clemens Losbichler
 | Dependencies: sudoku.h, exact_cover.h, any malloc implementation
 ----------------------*/

#ifndef SUS_H
#define SUS_H

#include "sudoku.h"

#define EXACT_COVER_IMPLEMENTATION
#include "exact_cover.h"

#ifndef size_t
typedef __SIZE_TYPE__ size_t;
#endif // size_t

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif // uint8_t

// Link these with some implementation
void *malloc(size_t size);
void free(void *p);

/* 
 Solves and fills sudoku with first possible solution. 
*/
int sus_solve_sudoku(Sudoku *s);

/* 
 Counts all possible solutions and does not fill sudoku. 
 Returns 1 in case of a solvable and 0 in case of an 
 unsolvable sudoku field. 
*/
unsigned long sus_count_solutions(Sudoku s);

/*
 Generates a sudoku field with according size and block_size 
 and fills it with the amount of hints. The sudoku is 
 always solvable (has exactly 1 solution).
*/
Sudoku sus_generate_sudoku(int size, int block_size, int hints);

#endif // SUS_H

#ifdef SUS_IMPLEMENTATION
#ifndef SUS_IMPLEMENTED
#define SUS_IMPLEMENTED

#include "sudoku.h"

uint8_t** sus_create_empty_sudoku_constraints(int size, int block_size, int *no_constraints, int *no_columns)
{
    *no_constraints = size * size * size;
    *no_columns = 4 * size * size;
    uint8_t **sets = malloc(*no_constraints * sizeof(uint8_t*));
    for (int i = 0; i < *no_constraints; i++) {
        sets[i] = malloc(*no_columns * sizeof(uint8_t));
        for (int j=0; j<*no_columns; j++) sets[i][j] = 0;
    }
    
    int cell_off  = 0;
    int row_off   = 1 * size * size;
    int col_off   = 2 * size * size;
    int block_off = 3 * size * size;

    // cell constraints
    for (int i=0; i<size*size; i++) {
        for (int j=0; j<size*size; j++) {
            for (int k=0; k<size; k++) {
                if (i == j)
                    sets[j*size + k][cell_off + i] = 1;
            }
        }
    }

    // row constraints
    for (int i = 0; i < size * size; i++) {
        for (int j = 0; j < size * size; j++) {
            for (int k = 0; k < size; k++) {
                if (j == k + i - i%size)
                    sets[i*size + k][row_off + j] = 1;
            }
        }
    }

    // column constraints
    for (int i = 0; i < size * size; i++) {
        for (int j = 0; j < size * size; j++) {
            for (int k = 0; k < size; k++) {
                if (j == (i*size + k) % (size*size))
                    sets[i*size + k][col_off + j] = 1;
            }
        }
    } 

    // block constraints
    for (int bx=0; bx<size/block_size; bx++) {
        for (int by=0; by<size/block_size; by++) {
            for (int n=0; n<size; n++) {
                for (int i=0; i<block_size; i++) {
                    for (int j=0; j<block_size; j++) {
                        int cell_x = bx * block_size + i;
                        int cell_y = by * block_size + j;
                        int row = cell_x * size * size + cell_y * size + n;
                        int col = block_off + n + bx * size * block_size + by * size;
                        sets[row][col] = 1;
                    }
                }
            }
        }
    }

    return sets;
}

DLXColumn *sus_create_sudoku_constraints(Sudoku s)
{
    int no_constraints, no_columns;
    uint8_t **constraints = sus_create_empty_sudoku_constraints(s.size, s.block_size, &no_constraints, &no_columns);
    DLXColumn *root = exact_constraints_to_dlx(constraints, no_constraints, no_columns);

    for (int i=0; i<s.size; i++) {
        for (int j=0; j<s.size; j++) {
            int val = s.field[i][j];
            if (val == 0) continue;
            int set_id = i * s.size * s.size + j * s.size + val - 1;
            exact_delete_possibility(root, set_id);
        }
    }

    return root;
}

int sus_solve_sudoku(Sudoku *s)
{
    if (!sudoku_is_valid(*s)) return 0;
    DLXColumn *root = sus_create_sudoku_constraints(*s);
    SetCover cover = {0};
    ExactCoverProblem solution = exact_solve(root, &cover, 1, -1UL);
    if (solution.solutions == 0) return 0;

    for (size_t k = 0; k < cover.count; k++) {
        int id = cover.items[k];

        int x = id / (s->size * s->size);
        int y = (id % (s->size * s->size)) / s->size;
        int val = id % s->size + 1;
        s->field[x][y] = val;
    }
    return 1;
}

unsigned long sus_count_solutions_until(Sudoku s, int max_solutions, int dynamic_programming)
{
    if (!sudoku_is_valid(s)) return 0;
    DLXColumn *root = sus_create_sudoku_constraints(s);
    exact_create_setcover_hashtable(s.size * s.size);
    SetCover cover = {0};
    if (dynamic_programming) return exact_solve_with_dp(root, &cover, 0, max_solutions).solutions;
    else                     return exact_solve(root, &cover, 0, max_solutions).solutions;
}

unsigned long sus_count_solutions(Sudoku s)
{
    return sus_count_solutions_until(s, 10000, 1);
}

uint8_t pseudo_rand(int max) {
    static long state = 1;
    state = (state * 12345 + 12345) % 3486509;
    return ((uint8_t) state) % max;
}

Sudoku sus_generate_sudoku(int size, int block_size, int hints)
{
    Sudoku s = sudoku_create_empty(size, block_size);
    int solutions = 0;
    // TODO: rework
    int equal_iterations = 0;
    while (solutions != 1UL && hints > 0 && equal_iterations < 100) {
        int x    = pseudo_rand(size);
        int y    = pseudo_rand(size);
        uint8_t val = pseudo_rand(block_size * block_size) + 1;
        
        if (s.field[x][y] == 0) {
            s.field[x][y] = val;

            solutions = sus_count_solutions_until(s, 3, 1);
            if (solutions == 0) {
                s.field[x][y] = 0;
                equal_iterations++;
            } else {
                hints--;
                equal_iterations = 0;
            }
        }
    }
    return s;
}

#endif // SUS_IMPLEMENTED
#endif // SUS_IMPLEMENTATION