#define SUDOKU_IMPLEMENTATION
#include "sudoku.h"

#define SUS_IMPLEMENTATION
#include "sus.h"

#include "examples.h"

#ifndef size_t
typedef __SIZE_TYPE__ size_t;
#endif // size_t

#define WASM_EXPORT(name) \
    __attribute__((export_name(#name))) \
    name

void *malloc(size_t size);
void free(void *p);

void *WASM_EXPORT(walloc)(size_t size) {
    return malloc(size);
}

void WASM_EXPORT(wfree)(void* ptr) {
    free(ptr);
}

Sudoku* WASM_EXPORT(export_sudoku_create_empty)(int n, int b) {
    Sudoku *s = malloc(sizeof(Sudoku));
    *s = sudoku_create_empty(n, b);
    return s;
}

Sudoku* WASM_EXPORT(export_sudoku_clone)(Sudoku s) {
    Sudoku *clone = malloc(sizeof(Sudoku));
    *clone = sudoku_clone(s);
    return clone;
}

Sudoku* WASM_EXPORT(export_generate_sudoku)(int size, int block_size, int hints) { 
    Sudoku *s = malloc(sizeof(Sudoku));
    *s = sus_generate_sudoku(size, block_size, hints); 
    return s;
}

int WASM_EXPORT(export_sudoku_is_valid)(Sudoku s)      { return sudoku_is_valid(s); }
int WASM_EXPORT(export_sudoku_is_solvable)(Sudoku s)   { return sudoku_is_solvable(s); }
int WASM_EXPORT(export_sudoku_get_solutions)(Sudoku s) { return sudoku_get_solutions(s); }

Sudoku* WASM_EXPORT(export_sudoku_example_easy)()      { Sudoku* s = malloc(sizeof(Sudoku)); sudoku_example_easy(s); return s; }
Sudoku* WASM_EXPORT(export_sudoku_example_medium)()    { Sudoku* s = malloc(sizeof(Sudoku)); sudoku_example_medium(s); return s; }
Sudoku* WASM_EXPORT(export_sudoku_example_hard)()      { Sudoku* s = malloc(sizeof(Sudoku)); sudoku_example_hard(s); return s; }
Sudoku* WASM_EXPORT(export_sudoku_example_very_hard)() { Sudoku* s = malloc(sizeof(Sudoku)); sudoku_example_very_hard(s); return s; }
Sudoku* WASM_EXPORT(export_sudoku_example_multiple_solutions)() { Sudoku* s = malloc(sizeof(Sudoku)); sudoku_example_multiple_solutions(s); return s; }

int WASM_EXPORT(export_solve_sudoku)(Sudoku* s)        { return sus_solve_sudoku(s); }
int WASM_EXPORT(export_count_solutions)(Sudoku* s)     { return sus_count_solutions(*s); }