void copy_board_to_sudoku(char board[9][9], Sudoku s) 
{
    int N = 9;
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            s.field[i][j] = board[i][j];
        }
    }
}

#define sudoku_example_easy(s)                            \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        static char board[9][9] = {                       \
            { 5,3,4, 6,0,8, 9,1,2 },                      \
            { 6,0,0, 0,9,5, 3,0,0 },                      \
            { 0,0,8, 0,4,0, 5,6,7 },                      \
            { 0,0,0, 0,6,0, 0,0,3 },                      \
            { 4,0,0, 8,0,3, 0,0,1 },                      \
            { 7,0,0, 0,2,0, 8,0,6 },                      \
            { 9,6,1, 0,0,0, 2,8,0 },                      \
            { 0,8,0, 4,0,9, 6,0,5 },                      \
            { 3,0,0, 0,8,0, 1,7,9 },                      \
        };                                                \
        copy_board_to_sudoku(board, *s);                  \
    } while (0)

#define sudoku_example_medium(s)                          \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        static char board[9][9] = {                       \
            { 5,3,4, 6,0,8, 0,1,2 },                      \
            { 0,7,0, 1,0,5, 0,4,0 },                      \
            { 1,9,8, 0,4,0, 5,6,0 },                      \
            { 8,0,0, 0,6,0, 4,0,3 },                      \
            { 4,0,0, 8,0,3, 7,0,1 },                      \
            { 7,0,3, 0,2,4, 8,0,0 },                      \
            { 9,6,1, 0,0,0, 2,0,0 },                      \
            { 2,8,7, 4,1,9, 6,3,5 },                      \
            { 3,0,0, 0,8,0, 1,7,9 },                      \
        };                                                \
        copy_board_to_sudoku(board, *s);                  \
    } while (0)

#define sudoku_example_very_hard(s)                       \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        static char board[9][9] = {                       \
            { 0,0,0, 0,0,9, 0,0,0 },                      \
            { 4,7,0, 0,0,0, 0,2,0 },                      \
            { 0,5,0, 0,0,1, 0,0,0 },                      \
            { 0,0,0, 0,0,0, 0,0,3 },                      \
            { 0,0,0, 7,5,0, 0,0,0 },                      \
            { 0,0,0, 0,0,0, 6,0,9 },                      \
            { 3,0,9, 0,0,8, 0,0,0 },                      \
            { 0,0,0, 2,0,0, 0,5,0 },                      \
            { 0,0,1, 0,0,0, 0,0,0 },                      \
        };                                                \
        copy_board_to_sudoku(board, *s);                  \
    } while (0)

#define sudoku_example_hard(s)                            \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        static char board[9][9] = {                       \
            { 0,2,6, 0,0,0, 0,0,3 },                      \
            { 0,0,0, 0,0,0, 0,0,0 },                      \
            { 7,0,0, 0,4,8, 0,6,0 },                      \
            { 0,7,0, 0,0,1, 0,0,0 },                      \
            { 0,4,0, 2,0,5, 0,0,9 },                      \
            { 2,0,0, 0,8,0, 0,5,7 },                      \
            { 5,0,8, 0,0,0, 0,0,0 },                      \
            { 0,0,0, 0,0,0, 0,2,0 },                      \
            { 0,0,0, 0,1,0, 5,4,0 },                      \
        };                                                \
        copy_board_to_sudoku(board, *s);                  \
    } while (0)

#define sudoku_example_multiple_solutions(s)              \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        static char board[9][9] = {                       \
            { 0,0,6, 0,0,0, 0,0,3 },                      \
            { 0,0,0, 0,0,0, 0,0,0 },                      \
            { 7,0,0, 0,4,8, 0,6,0 },                      \
            { 0,7,0, 0,0,1, 0,0,0 },                      \
            { 0,4,0, 2,0,5, 0,0,9 },                      \
            { 2,0,0, 0,8,0, 0,5,7 },                      \
            { 5,0,8, 0,0,0, 0,0,0 },                      \
            { 0,0,0, 0,0,0, 0,2,0 },                      \
            { 0,0,0, 0,1,0, 5,4,0 },                      \
        };                                                \
        copy_board_to_sudoku(board, *s);                  \
    } while (0)

#define sudoku_example_no_solutions(s)                    \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        static char board[9][9] = {                       \
            { 1,0,6, 0,0,0, 0,0,3 },                      \
            { 0,1,0, 0,0,0, 0,0,0 },                      \
            { 7,0,1, 0,4,8, 0,6,0 },                      \
            { 0,7,0, 1,0,0, 0,0,0 },                      \
            { 0,4,0, 2,1,5, 0,0,9 },                      \
            { 2,0,0, 0,8,1, 0,5,7 },                      \
            { 5,0,8, 0,3,0, 1,0,0 },                      \
            { 0,0,0, 0,0,0, 3,1,0 },                      \
            { 3,0,0, 0,0,0, 5,4,1 },                      \
        };                                                \
        copy_board_to_sudoku(board, *s);                  \
    } while (0)

#define sudoku_example_wrong(s)                           \
    do                                                    \
    {                                                     \
        *(s) = (Sudoku) sudoku_create_empty(9, 3);        \
        static char board[9][9] = {                       \
            { 1,2,3, 4,7,9, 8,6,5 },                      \
            { 4,7,6, 3,8,5, 9,2,1 },                      \
            { 9,5,8, 6,2,1, 7,3,4 },                      \
            { 2,8,1, 9,4,6, 5,7,3 },                      \
            { 6,9,4, 7,5,3, 1,8,2 },                      \
            { 7,3,5, 8,1,2, 6,4,9 },                      \
            { 3,4,9, 5,6,8, 2,1,7 },                      \
            { 8,1,7, 2,9,4, 3,5,6 },                      \
            { 5,6,2, 1,3,7, 4,9,2 },                      \
        };                                                \
        copy_board_to_sudoku(board, *s);                  \
    } while (0)
