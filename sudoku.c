#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "example_sudokus.h"

#define N 9
#define C 3

char** create_empty_sudoku()
{
    char** s = malloc(sizeof(int*) * N);
    if (s == NULL) {
        printf("Malloc error\n");
        exit(-1);
    }

    for (int i=0; i<N; i++) {
        s[i] = malloc(sizeof(int) * N);
        for (int j=0; j<N; j++) {
            s[i][j] = 0;
        }
    }
    return s;
}

void print_sudoku(char** s)
{
    for (int i=0; i<N; i++) {
        if (i % C == 0) {
            printf("|");
            for (int k=0; k<N+C/2; k++) printf("--");
            printf("|\n");
        }
        for (int j=0; j<N; j++) {
            if (j % C == 0) printf("|");
            printf("|");
            if (s[i][j] == 0)
                printf(" ");
            else
                printf("%d", s[i][j]);
        }
        printf("|\n");
    }
    printf("|");
    for (int k=0; k<N+C/2; k++) printf("--");
    printf("|\n");
}

bool check_sudoku(char** s) 
{
    for (int i=0; i<N; i++) {
        int row[10] = {0};
        for (int j=0; j<N; j++) {
            int val = s[i][j];
            row[val] += 1;
            if (val != 0 && row[val] > 1) {
                return false;
            }
        }
    }    
    
    for (int i=0; i<N; i++) {
        int col[10] = {0};
        for (int j=0; j<N; j++) {
            int val = s[j][i];
            col[val] += 1;
            if (val != 0 && col[val] > 1) {
                return false;
            }
        }
    }
    
    for (int i=0; i<N/C; i++) {
        for (int j=0; j<N/C; j++) {
            int cell[10] = {0};
            for (int m=0; m<C; m++) {
                for (int n=0; n<C; n++) {
                    int val = s[C * i + m][C * j + n]; 
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

int all_solutions(char** s) 
{
    if (!check_sudoku(s)) {
        return 0;
    }

    int solutions = 0;
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            if (s[i][j] == 0) {
                for (int d=1; d<=9; d++) {
                    s[i][j] = d;
                    solutions += all_solutions(s);
                    s[i][j] = 0;
                }
                return solutions;
            }
        }
    }
    return 1;
}

bool is_solvable(char** s) 
{
    if (!check_sudoku(s)) {
        return false;
    }

    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            if (s[i][j] == 0) {
                for (int d=1; d<=9; d++) {
                    s[i][j] = d;
                    if (is_solvable(s)) {
                        return true;
                    }
                    s[i][j] = 0;
                }
                return false;
            }
        }
    }
    return true;
}

void measure_solver(char** s) 
{
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int solutions = all_solutions(s);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("found %d solutions in %.2fs\n", solutions, cpu_time_used);
}

char** create_sudoku(int (rand)(int n), int hints) 
{
    char** s = create_empty_sudoku();
    while (hints > 0) {
        int x = rand(N);       
        int y = rand(N);
        char val = rand(9) + 1;
        if (s[x][y] == 0) {
            s[x][y] = val;
            if (!check_sudoku(s)) {
                s[x][y] = 0;
            } else {
                hints--;
            }
        }
    }
    return s;
}

int pseudorandom(int n) 
{
    return rand() % n;
}


int main() 
{
    srand(time(NULL));

    char** s;
    char** solved = create_empty_sudoku();
    int attempts = 0;
    while (attempts < 10) {
        attempts++;
        s = create_sudoku(pseudorandom, 17);
        printf("checking if solvable..\n");
        for (int i=0; i<N; i++) {
            for (int j=0; j<N; j++) {
                solved[i][j] = s[i][j];
            }
        }
        if (is_solvable(solved)) break;
        printf("attempt %d\n", attempts);
    }
    print_sudoku(s);
    print_sudoku(solved);

    /*
    easy_sudoku
    printf("--easy--\n");
    measure_solver(sudoku);
    
    medium_sudoku
    printf("--medium--\n");
    measure_solver(sudoku);
 
    hard_sudoku
    printf("--hard--\n");
    measure_solver(sudoku);
    
    very_hard_sudoku
    printf("--very hard--\n");
    measure_solver(sudoku);

    non_proper_sudoku
    printf("--non proper--\n");
    measure_solver(sudoku);
    */
 
    return 0;
}