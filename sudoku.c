#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

int N = 9;
int C = 3;
int** sudoku = NULL;

void empty_sudoku()
{
    if (sudoku != NULL) free(sudoku);
    sudoku = malloc(sizeof(int*) * N);
    if (sudoku == NULL) {
        printf("Malloc error\n");
        exit(-1);
    }

    for (int i=0; i<N; i++) {
        sudoku[i] = malloc(sizeof(int) * N);
        for (int j=0; j<N; j++) {
            sudoku[i][j] = 0;
        }
    }
}

#define very_hard_sudoku \
    empty_sudoku(); \
    sudoku[0] = (int[]) { 0,2,6, 0,0,0, 0,0,3 }; \
    sudoku[1] = (int[]) { 0,0,0, 0,0,0, 0,0,0 }; \
    sudoku[2] = (int[]) { 7,0,0, 0,4,8, 0,6,0 }; \
    sudoku[3] = (int[]) { 0,7,0, 0,0,1, 0,0,0 }; \
    sudoku[4] = (int[]) { 0,4,0, 2,0,5, 0,0,9 }; \
    sudoku[5] = (int[]) { 2,0,0, 0,8,0, 0,5,7 }; \
    sudoku[6] = (int[]) { 5,0,8, 0,0,0, 0,0,0 }; \
    sudoku[7] = (int[]) { 0,0,0, 0,0,0, 0,2,0 }; \
    sudoku[8] = (int[]) { 0,0,0, 0,1,0, 5,4,0 };

#define hard_sudoku \
    empty_sudoku(); \
    sudoku[0] = (int[]) { 5,3,0, 0,7,0, 0,0,0 }; \
    sudoku[1] = (int[]) { 6,0,0, 1,9,5, 0,0,0 }; \
    sudoku[2] = (int[]) { 0,9,8, 0,0,0, 0,6,0 }; \
    sudoku[3] = (int[]) { 8,0,0, 0,6,0, 0,0,3 }; \
    sudoku[4] = (int[]) { 4,0,0, 8,0,3, 0,0,1 }; \
    sudoku[5] = (int[]) { 7,0,0, 0,2,0, 0,0,6 }; \
    sudoku[6] = (int[]) { 0,6,0, 0,0,0, 2,8,0 }; \
    sudoku[7] = (int[]) { 0,0,0, 4,1,9, 0,0,5 }; \
    sudoku[8] = (int[]) { 0,0,0, 0,8,0, 0,7,9 };

#define medium_sudoku \
    empty_sudoku(); \
    sudoku[0] = (int[]) { 5,3,4, 6,7,8, 9,1,2 }; \
    sudoku[1] = (int[]) { 6,7,2, 1,9,5, 3,0,0 }; \
    sudoku[2] = (int[]) { 0,9,8, 0,4,0, 5,6,7 }; \
    sudoku[3] = (int[]) { 0,0,0, 0,6,0, 4,0,3 }; \
    sudoku[4] = (int[]) { 4,0,0, 8,0,3, 0,0,1 }; \
    sudoku[5] = (int[]) { 7,0,0, 0,2,0, 8,0,6 }; \
    sudoku[6] = (int[]) { 9,6,1, 0,0,0, 2,8,0 }; \
    sudoku[7] = (int[]) { 0,8,0, 4,1,9, 6,0,5 }; \
    sudoku[8] = (int[]) { 3,0,0, 0,8,0, 1,7,9 };

#define easy_sudoku \
    empty_sudoku(); \
    sudoku[0] = (int[]) { 5,3,4, 6,7,8, 9,1,2 }; \
    sudoku[1] = (int[]) { 6,7,2, 1,9,5, 3,4,8 }; \
    sudoku[2] = (int[]) { 1,9,8, 0,4,0, 5,6,7 }; \
    sudoku[3] = (int[]) { 8,0,0, 0,6,0, 4,0,3 }; \
    sudoku[4] = (int[]) { 4,0,0, 8,0,3, 7,0,1 }; \
    sudoku[5] = (int[]) { 7,0,3, 0,2,4, 8,0,6 }; \
    sudoku[6] = (int[]) { 9,6,1, 0,0,0, 2,8,4 }; \
    sudoku[7] = (int[]) { 2,8,7, 4,1,9, 6,3,5 }; \
    sudoku[8] = (int[]) { 3,0,0, 0,8,0, 1,7,9 };

void print_sudoku()
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
            if (sudoku[i][j] == 0)
                printf(" ");
            else
                printf("%d", sudoku[i][j]);
        }
        printf("|\n");
    }
    printf("|");
    for (int k=0; k<N+C/2; k++) printf("--");
    printf("|\n");
}

bool check_sudoku(int** s) 
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
        int cell[10] = {0};
        for (int j=0; j<C; j++) {
            for (int k=0; k<C; k++) {
                int val = s[C * i + j][C * i + k]; 
                cell[val] += 1;
                if (val != 0 && cell[val] > 1) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool backtracking(int** s, int depth) 
{
    if (!check_sudoku(s)) {
        return false;
    }

    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            if (s[i][j] == 0) {
                for (int d=1; d<=9; d++) {
                    s[i][j] = d;
                    if (backtracking(s, depth+1)) {
                        return true;
                    }
                    s[i][j] = 0;
                }
                return false;
            }
        }
    }
}

int main() 
{
    clock_t start, end;
    double cpu_time_used;

    easy_sudoku
    start = clock();
    if (!backtracking(sudoku, 0)) printf("no solution\n");
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("easy = %.2fs\n", cpu_time_used);
    
    medium_sudoku
    start = clock();
    if (!backtracking(sudoku, 0)) printf("no solution\n");
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("medium = %.2fs\n", cpu_time_used);
    
    hard_sudoku
    start = clock();
    if (!backtracking(sudoku, 0)) printf("no solution\n");
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("hard = %.2fs\n", cpu_time_used);

    very_hard_sudoku
    start = clock();
    if (!backtracking(sudoku, 0)) printf("no solution\n");
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("hard = %.2fs\n", cpu_time_used);
    
    print_sudoku();
    return 0;
}