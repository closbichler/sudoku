
#define sudoku_example_medium(s)                            \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        (s)->field[0] = (char[]) { 5,3,4, 6,0,8, 0,1,2 }; \
        (s)->field[1] = (char[]) { 0,7,0, 1,0,5, 0,4,0 }; \
        (s)->field[2] = (char[]) { 1,9,8, 0,4,0, 5,6,0 }; \
        (s)->field[3] = (char[]) { 8,0,0, 0,6,0, 4,0,3 }; \
        (s)->field[4] = (char[]) { 4,0,0, 8,0,3, 7,0,1 }; \
        (s)->field[5] = (char[]) { 7,0,3, 0,2,4, 8,0,0 }; \
        (s)->field[6] = (char[]) { 9,6,1, 0,0,0, 2,0,0 }; \
        (s)->field[7] = (char[]) { 2,8,7, 4,1,9, 6,3,5 }; \
        (s)->field[8] = (char[]) { 3,0,0, 0,8,0, 1,7,9 }; \
    } while (0)

#define sudoku_example_easy(s)                        \
    do                                                  \
    {                                                   \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);      \
        (s)->field[0] = (char[]) { 5,3,4, 6,0,8, 9,1,2 }; \
        (s)->field[1] = (char[]) { 6,0,0, 0,9,5, 3,0,0 }; \
        (s)->field[2] = (char[]) { 0,0,8, 0,4,0, 5,6,7 }; \
        (s)->field[3] = (char[]) { 0,0,0, 0,6,0, 0,0,3 }; \
        (s)->field[4] = (char[]) { 4,0,0, 8,0,3, 0,0,1 }; \
        (s)->field[5] = (char[]) { 7,0,0, 0,2,0, 8,0,6 }; \
        (s)->field[6] = (char[]) { 9,6,1, 0,0,0, 2,8,0 }; \
        (s)->field[7] = (char[]) { 0,8,0, 4,0,9, 6,0,5 }; \
        (s)->field[8] = (char[]) { 3,0,0, 0,8,0, 1,7,9 }; \
    } while (0)

#define sudoku_example_very_hard(s)                          \
    do                                                  \
    {                                                   \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);      \
        (s)->field[0] = (char[]) { 0,0,0, 0,0,9, 0,0,0 }; \
        (s)->field[1] = (char[]) { 4,7,0, 0,0,0, 0,2,0 }; \
        (s)->field[2] = (char[]) { 0,5,0, 0,0,1, 0,0,0 }; \
        (s)->field[3] = (char[]) { 0,0,0, 0,0,0, 0,0,3 }; \
        (s)->field[4] = (char[]) { 0,0,0, 7,5,0, 0,0,0 }; \
        (s)->field[5] = (char[]) { 0,0,0, 0,0,0, 6,0,9 }; \
        (s)->field[6] = (char[]) { 3,0,9, 0,0,8, 0,0,0 }; \
        (s)->field[7] = (char[]) { 0,0,0, 2,0,0, 0,5,0 }; \
        (s)->field[8] = (char[]) { 0,0,1, 0,0,0, 0,0,0 }; \
    } while (0)

#define sudoku_example_hard(s)                          \
    do                                                  \
    {                                                   \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);      \
        (s)->field[0] = (char[]) { 0,2,6, 0,0,0, 0,0,3 }; \
        (s)->field[1] = (char[]) { 0,0,0, 0,0,0, 0,0,0 }; \
        (s)->field[2] = (char[]) { 7,0,0, 0,4,8, 0,6,0 }; \
        (s)->field[3] = (char[]) { 0,7,0, 0,0,1, 0,0,0 }; \
        (s)->field[4] = (char[]) { 0,4,0, 2,0,5, 0,0,9 }; \
        (s)->field[5] = (char[]) { 2,0,0, 0,8,0, 0,5,7 }; \
        (s)->field[6] = (char[]) { 5,0,8, 0,0,0, 0,0,0 }; \
        (s)->field[7] = (char[]) { 0,0,0, 0,0,0, 0,2,0 }; \
        (s)->field[8] = (char[]) { 0,0,0, 0,1,0, 5,4,0 }; \
    } while (0)

#define sudoku_example_non_proper(s)                      \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        (s)->field[0] = (char[]) { 0,0,6, 0,0,0, 0,0,3 }; \
        (s)->field[1] = (char[]) { 0,0,0, 0,0,0, 0,0,0 }; \
        (s)->field[2] = (char[]) { 7,0,0, 0,4,8, 0,6,0 }; \
        (s)->field[3] = (char[]) { 0,7,0, 0,0,1, 0,0,0 }; \
        (s)->field[4] = (char[]) { 0,4,0, 2,0,5, 0,0,9 }; \
        (s)->field[5] = (char[]) { 2,0,0, 0,8,0, 0,5,7 }; \
        (s)->field[6] = (char[]) { 5,0,8, 0,0,0, 0,0,0 }; \
        (s)->field[7] = (char[]) { 0,0,0, 0,0,0, 0,2,0 }; \
        (s)->field[8] = (char[]) { 0,0,0, 0,1,0, 5,4,0 }; \
    } while (0)
