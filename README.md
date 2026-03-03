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

## Resources
- https://en.wikipedia.org/wiki/Exact_cover#Sudoku
- https://en.wikipedia.org/wiki/Knuth%27s_Algorithm_X
- https://sudokugarden.de/de/info/minimal
- https://github.com/wingo/walloc
- https://github.com/rob-blackbourn/example-wasm-array-passing