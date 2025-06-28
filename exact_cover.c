#include <stdio.h>

int main()
{
    int N = 9;
    char row_column[N*N][N];

    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            for (int k=0; k<N; k++) {
                if (i == j)
                    row_column[k + j*N][i] = 1;
                else
                    row_column[k + j*N][i] = 0;
            }
        }
    }

    char row_number[N*N][N];

    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            for (int k=0; k<N; k++) {
                if (i == k)
                    row_number[k + j*N][i] = 1;
                else
                    row_number[k + j*N][i] = 0;
            }
        }
    }

    char column_number[N*N][N];

    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            for (int k=0; k<N; k++) {
                if (i == k)
                    column_number[k + j*N][i] = 1;
                else
                    column_number[k + j*N][i] = 0;
            }
        }
    }

    char box_number[N*N][N];

    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            for (int k=0; k<N; k++) {
                if (i == k)
                    box_number[k + j*N][i] = 1;
                else
                    box_number[k + j*N][i] = 0;
            }
        }
    }

    int num_sets = N*N*4;
    int num_cols = N*N*N;
    char sets[num_sets][num_cols];

    for (int i=0; i<N*N; i++) {
        
    }

    for (int i=0; i<N*N; i++) {
        for (int j=0; j<N; j++) {
            printf("%d ", row_number[i][j]);
        }
        printf("\n");
    }

    return 0;
}