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
./exact_cover_test && ./sudoku_test
```

## Implementation
The implementation contains one simple recursive greedy algorithm and a reduction from sudoku to the exact cover problem as described by Donald Knuth (also called _Algorithm X_).

## TODO
- free sudoku resources (currently leaking everything)
- sudoku generation
- refactor hashing
    - Hash quality is weak: HASH_CONST is 2 at exact_cover.h:56, which increases collisions and lowers lookup efficiency.
- actually usable UI

## Resources
- https://en.wikipedia.org/wiki/Exact_cover#Sudoku
- https://en.wikipedia.org/wiki/Knuth%27s_Algorithm_X
- https://sudokugarden.de/de/info/minimal
- https://github.com/wingo/walloc
- https://github.com/rob-blackbourn/example-wasm-array-passing
- https://blog.wolfram.com/2020/06/02/using-integer-optimization-to-build-and-solve-sudoku-games-with-the-wolfram-language%C2%A0/
- https://github.com/kevquirk/simple.css