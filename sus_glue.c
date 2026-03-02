#define SUS_IMPLEMENTATION
#include "sus.h"

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

char* WASM_EXPORT(get_kek)() {
    return "kek";
}

int* WASM_EXPORT(get_arr)() {
    int* kek = malloc(sizeof(int) * 3);
    kek[0] = 0;
    kek[1] = 1;
    kek[2] = 2;
    return kek;
}