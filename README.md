# Sudoku creator and solver
Create and solve sudokus with different algorithmic approaches.

## Build and Run
```
make web
python3 -m http.server 8080
```

## Test Suite
```
make test
./sudoku_test with_generator
```

## Implementation
The implementation contains one simple recursive greedy algorithm and a reduction from sudoku to the exact cover problem as described by Donald Knuth (also called _Algorithm X_).

## TODO
- free sudoku resources (currently leaking everything)
- sudoku generation
- dynamic programming in sus_dlx_solve_exact_cover to increase performance:
    - Idea: hash the input (DLXColumn *root, SetCover *cover, int find_first_solution_only) and save the number of solutions if already calculated
    - How to hash the entire matrix, starting fromm the root? Hash all pointers? => O(#all nodes in matrix)
    - Or hash just the setcover? that should determine the matrix uniquely anyways.
    - First: check if the multiple calculations are even a relevant number
- actually usable UI

## Resources
- https://en.wikipedia.org/wiki/Exact_cover#Sudoku
- https://en.wikipedia.org/wiki/Knuth%27s_Algorithm_X
- https://sudokugarden.de/de/info/minimal
- https://github.com/wingo/walloc
- https://github.com/rob-blackbourn/example-wasm-array-passing
- https://blog.wolfram.com/2020/06/02/using-integer-optimization-to-build-and-solve-sudoku-games-with-the-wolfram-language%C2%A0/