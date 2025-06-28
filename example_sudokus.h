char** sudoku;

#define non_proper_sudoku \
    sudoku = create_empty_sudoku(); \
    sudoku[0] = (char[]) { 0,0,6, 0,0,0, 0,0,3 }; \
    sudoku[1] = (char[]) { 0,0,0, 0,0,0, 0,0,0 }; \
    sudoku[2] = (char[]) { 7,0,0, 0,4,8, 0,6,0 }; \
    sudoku[3] = (char[]) { 0,7,0, 0,0,1, 0,0,0 }; \
    sudoku[4] = (char[]) { 0,4,0, 2,0,5, 0,0,9 }; \
    sudoku[5] = (char[]) { 2,0,0, 0,8,0, 0,5,7 }; \
    sudoku[6] = (char[]) { 5,0,8, 0,0,0, 0,0,0 }; \
    sudoku[7] = (char[]) { 0,0,0, 0,0,0, 0,2,0 }; \
    sudoku[8] = (char[]) { 0,0,0, 0,1,0, 5,4,0 };

#define very_hard_sudoku \
    sudoku = create_empty_sudoku(); \
    sudoku[0] = (char[]) { 0,2,6, 0,0,0, 0,0,3 }; \
    sudoku[1] = (char[]) { 0,0,0, 0,0,0, 0,0,0 }; \
    sudoku[2] = (char[]) { 7,0,0, 0,4,8, 0,6,0 }; \
    sudoku[3] = (char[]) { 0,7,0, 0,0,1, 0,0,0 }; \
    sudoku[4] = (char[]) { 0,4,0, 2,0,5, 0,0,9 }; \
    sudoku[5] = (char[]) { 2,0,0, 0,8,0, 0,5,7 }; \
    sudoku[6] = (char[]) { 5,0,8, 0,0,0, 0,0,0 }; \
    sudoku[7] = (char[]) { 0,0,0, 0,0,0, 0,2,0 }; \
    sudoku[8] = (char[]) { 0,0,0, 0,1,0, 5,4,0 };

#define hard_sudoku \
    sudoku = create_empty_sudoku(); \
    sudoku[0] = (char[]) { 5,3,0, 0,7,0, 0,0,0 }; \
    sudoku[1] = (char[]) { 6,0,0, 1,9,5, 0,0,0 }; \
    sudoku[2] = (char[]) { 0,9,8, 0,0,0, 0,6,0 }; \
    sudoku[3] = (char[]) { 8,0,0, 0,6,0, 0,0,3 }; \
    sudoku[4] = (char[]) { 4,0,0, 8,0,3, 0,0,1 }; \
    sudoku[5] = (char[]) { 7,0,0, 0,2,0, 0,0,6 }; \
    sudoku[6] = (char[]) { 0,6,0, 0,0,0, 2,8,0 }; \
    sudoku[7] = (char[]) { 0,0,0, 4,0,9, 0,0,5 }; \
    sudoku[8] = (char[]) { 0,0,0, 0,0,0, 0,7,9 };

#define medium_sudoku \
    sudoku = create_empty_sudoku(); \
    sudoku[0] = (char[]) { 5,3,4, 6,0,8, 9,1,2 }; \
    sudoku[1] = (char[]) { 6,0,0, 0,9,5, 3,0,0 }; \
    sudoku[2] = (char[]) { 0,0,8, 0,4,0, 5,6,7 }; \
    sudoku[3] = (char[]) { 0,0,0, 0,6,0, 0,0,3 }; \
    sudoku[4] = (char[]) { 4,0,0, 8,0,3, 0,0,1 }; \
    sudoku[5] = (char[]) { 7,0,0, 0,2,0, 8,0,6 }; \
    sudoku[6] = (char[]) { 9,6,1, 0,0,0, 2,8,0 }; \
    sudoku[7] = (char[]) { 0,8,0, 4,0,9, 6,0,5 }; \
    sudoku[8] = (char[]) { 3,0,0, 0,8,0, 1,7,9 };

#define easy_sudoku \
    sudoku = create_empty_sudoku(); \
    sudoku[0] = (char[]) { 5,3,4, 6,0,8, 0,1,2 }; \
    sudoku[1] = (char[]) { 0,7,0, 1,0,5, 0,4,0 }; \
    sudoku[2] = (char[]) { 1,9,8, 0,4,0, 5,6,0 }; \
    sudoku[3] = (char[]) { 8,0,0, 0,6,0, 4,0,3 }; \
    sudoku[4] = (char[]) { 4,0,0, 8,0,3, 7,0,1 }; \
    sudoku[5] = (char[]) { 7,0,3, 0,2,4, 8,0,0 }; \
    sudoku[6] = (char[]) { 9,6,1, 0,0,0, 2,0,0 }; \
    sudoku[7] = (char[]) { 2,8,7, 4,1,9, 6,3,5 }; \
    sudoku[8] = (char[]) { 3,0,0, 0,8,0, 1,7,9 };
