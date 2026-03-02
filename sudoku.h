/*----------------------
 | Suoku header; complementary to SUS
 |
 | Author: Clemens Losbichler
 | Dependencies: stdlib, stdio, string, stdbool
 ----------------------*/

#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdbool.h>

typedef struct {
    char** field;
    int size;
    int block_size;
} Sudoku;

Sudoku sudoku_create_empty(int size, int block_size);
Sudoku sudoku_create(int size, int block_size, int (rand)(int n), int hints);
Sudoku sudoku_clone(Sudoku s);

void   sudoku_print(Sudoku s);
bool   sudoku_is_valid(Sudoku s);
bool   sudoku_is_solvable(Sudoku s);
int    sudoku_get_solutions(Sudoku s);

#endif // SUDOKU_H

#ifdef SUDOKU_IMPLEMENTATION
#ifndef SUDOKU_IMPLEMENTED
#define SUDOKU_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

Sudoku sudoku_create_empty(int size, int block_size)
{
    Sudoku s = {
        .size = size,
        .block_size = block_size
    };
    s.field = (char**) malloc(sizeof(char*) * size);
    if (s.field == NULL) {
        fprintf(stderr, "Malloc error\n");
        exit(-1);
    }

    for (int i=0; i<s.size; i++) {
        s.field[i] = (char*) malloc(sizeof(char) * size);
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

void sudoku_print(Sudoku s)
{
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

bool sudoku_is_valid(Sudoku s) 
{
    for (int i=0; i<s.size; i++) {
        int row[s.size + 1];
        memset(row, 0, sizeof(row));
        for (int j=0; j<s.size; j++) {
            int val = s.field[i][j];
            row[val] += 1;
            if (val != 0 && row[val] > 1) {
                return false;
            }
        }
    }    
    
    for (int i=0; i<s.size; i++) {
        int col[s.size + 1];
        memset(col, 0, sizeof(col));
        for (int j=0; j<s.size; j++) {
            int val = s.field[j][i];
            col[val] += 1;
            if (val != 0 && col[val] > 1) {
                return false;
            }
        }
    }
    
    for (int i=0; i<s.size/s.block_size; i++) {
        for (int j=0; j<s.size/s.block_size; j++) {
            int cell[s.size + 1];
            memset(cell, 0, sizeof(cell));
            for (int m=0; m<s.block_size; m++) {
                for (int n=0; n<s.block_size; n++) {
                    int val = s.field[s.block_size * i + m][s.block_size * j + n]; 
                    cell[val] += 1;
                    if (val != 0 && cell[val] > 1) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// brute-force backtracking
int sudoku_get_solutions(Sudoku s) 
{
    if (!sudoku_is_valid(s)) return 0;
    int solutions = 0;
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

bool sudoku_is_solvable(Sudoku s) 
{
    if (!sudoku_is_valid(s)) return false;
    for (int i=0; i<s.size; i++) {
        for (int j=0; j<s.size; j++) {
            if (s.field[i][j] == 0) {
                for (int d=1; d<=s.size; d++) {
                    s.field[i][j] = d;
                    if (sudoku_is_solvable(s)) return true;
                    s.field[i][j] = 0;
                }
                return false;
            }
        }
    }
    return true;
}

#endif // SUDOKU_IMPLEMENTED
#endif // SUDOKU_IMPLEMENTATION
