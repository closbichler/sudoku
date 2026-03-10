/*----------------------
 | Suoku header; complementary to SUS
 |
 | Author: Clemens Losbichler
 | Dependencies: any malloc implementation
 ----------------------*/

#ifndef SUDOKU_H
#define SUDOKU_H

#ifndef NULL
#define NULL ((void*)0)
#endif // NULL

#ifndef size_t
typedef __SIZE_TYPE__ size_t;
typedef unsigned long int ulong;
#endif // size_t

// Link these with some implementation
void *malloc(size_t size);
void free(void *p);

typedef struct {
    int size;
    int block_size;
    char** field;
} Sudoku;

Sudoku sudoku_create_empty(int size, int block_size);
Sudoku sudoku_create(int size, int block_size, int (rand)(int n), int hints);
Sudoku sudoku_clone(Sudoku s);

int    sudoku_is_valid(Sudoku s);
int    sudoku_is_solvable(Sudoku s);
ulong  sudoku_get_solutions(Sudoku s);

#endif // SUDOKU_H

#ifdef SUDOKU_IMPLEMENTATION
#ifndef SUDOKU_IMPLEMENTED
#define SUDOKU_IMPLEMENTED

Sudoku sudoku_create_empty(int size, int block_size)
{
    Sudoku s = {
        .size = size,
        .block_size = block_size
    };
    s.field = (char**) malloc(sizeof(char*) * size);
    if (s.field == NULL) {
        // Allocation failed
        return (Sudoku) { 0 };
    }

    for (int i=0; i<s.size; i++) {
        s.field[i] = (char*) malloc(sizeof(char) * size);
        if (s.field[i] == NULL) {
            // Allocation failed
            return (Sudoku) { 0 };
        }

        for (int j=0; j<s.size; j++) {
            s.field[i][j] = 0;
        }
    }
    return s;
}

Sudoku sudoku_create(int size, int block_size, int (rand)(int n), int hints) 
{
    Sudoku s = sudoku_create_empty(size, block_size);
    while (hints > 0) {
        int x = rand(s.size);       
        int y = rand(s.size);
        char val = rand(s.size) + 1;
        if (s.field[x][y] == 0) {
            s.field[x][y] = val;
            if (!sudoku_is_valid(s)) {
                s.field[x][y] = 0;
            } else {
                hints--;
            }
        }
    }
    return s;
}

Sudoku sudoku_clone(Sudoku s)
{
    Sudoku c = sudoku_create_empty(s.size, s.block_size);
    for (int i=0; i<s.size; i++) {
        for (int j=0; j<s.size; j++) {
            c.field[i][j] = s.field[i][j];
        }
    }
    return c;
}

void set_array_to_zero(int *arr, int size) 
{
    for (int i=0; i<size; i++) {
        arr[i] = 0;
    }
}

int sudoku_is_valid(Sudoku s) 
{
    for (int i=0; i<s.size; i++) {
        int row[s.size + 1]; 
        set_array_to_zero(row, s.size + 1);       
        for (int j=0; j<s.size; j++) {
            int val = s.field[i][j];
            if (val > s.size) return 0;
            row[val] += 1;
            if (val != 0 && row[val] > 1) {
                return 0;
            }
        }
    }    
    
    for (int i=0; i<s.size; i++) {
        int col[s.size + 1];
        set_array_to_zero(col, s.size + 1);
        for (int j=0; j<s.size; j++) {
            int val = s.field[j][i];
            col[val] += 1;
            if (val != 0 && col[val] > 1) {
                return 0;
            }
        }
    }
    
    for (int i=0; i<s.size/s.block_size; i++) {
        for (int j=0; j<s.size/s.block_size; j++) {
            int cell[s.size + 1];
            set_array_to_zero(cell, s.size + 1);
            for (int m=0; m<s.block_size; m++) {
                for (int n=0; n<s.block_size; n++) {
                    int val = s.field[s.block_size * i + m][s.block_size * j + n]; 
                    cell[val] += 1;
                    if (val != 0 && cell[val] > 1) {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

// brute-force backtracking
ulong sudoku_get_solutions(Sudoku s) 
{
    if (!sudoku_is_valid(s)) return 0;
    ulong solutions = 0;
    for (int i=0; i<s.size; i++) {
        for (int j=0; j<s.size; j++) {
            if (s.field[i][j] == 0) {
                for (int d=1; d<=s.size; d++) {
                    s.field[i][j] = d;
                    solutions += sudoku_get_solutions(s);
                    s.field[i][j] = 0;
                }
                return solutions;
            }
        }
    }
    return 1;
}

int sudoku_is_solvable(Sudoku s) 
{
    if (!sudoku_is_valid(s)) return 0;
    for (int i=0; i<s.size; i++) {
        for (int j=0; j<s.size; j++) {
            if (s.field[i][j] == 0) {
                for (int d=1; d<=s.size; d++) {
                    s.field[i][j] = d;
                    if (sudoku_is_solvable(s)) return 1;
                    s.field[i][j] = 0;
                }
                return 0;
            }
        }
    }
    return 1;
}

#endif // SUDOKU_IMPLEMENTED
#endif // SUDOKU_IMPLEMENTATION
