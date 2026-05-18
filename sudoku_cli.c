#include <stdio.h>
#include <stdbool.h>

#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"

#define SUS_IMPLEMENTATION
#include "sus.h"

bool sudoku_load(Sudoku *sudoku, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return false;
    }

    fscanf(file, "sudoku %d %d", &sudoku->size, &sudoku->block_size);
    *sudoku = sudoku_create_empty(sudoku->size, sudoku->block_size);

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (fscanf(file, "%hhd", &sudoku->field[i][j]) != 1) {
                fprintf(stderr, "Invalid format in sudoku file\n");
                fclose(file);
                return false;
            }
        }
    }

    fclose(file);
    return true;
}

void sudoku_print(Sudoku s)
{
    if (s.field == NULL) { fprintf(stdout, "empty sudoku\n"); return; }
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <sudoku_file>\n", argv[0]);
        return 1;
    }

    Sudoku sudoku;
    if (!sudoku_load(&sudoku, argv[1])) {
        fprintf(stderr, "Failed to load sudoku from file: %s\n", argv[1]);
        return 1;
    }


    printf("Original Sudoku:\n");
    sudoku_print(sudoku);

    if (sus_solve_sudoku(&sudoku)) {
        printf("Solved Sudoku:\n");
        sudoku_print(sudoku);
    } else {
        printf("No solution found for the given Sudoku.\n");
    }

    return 0;
}