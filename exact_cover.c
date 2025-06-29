#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define DA_INIT_CAP 256

#define da_reserve(da, expected_capacity)                                              \
    do {                                                                                   \
        if ((expected_capacity) > (da)->capacity) {                                        \
            if ((da)->capacity == 0) {                                                     \
                (da)->capacity = DA_INIT_CAP;                                          \
            }                                                                              \
            while ((expected_capacity) > (da)->capacity) {                                 \
                (da)->capacity *= 2;                                                       \
            }                                                                              \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                         \
        }                                                                                  \
    } while (0)

#define da_append(da, item)                \
    do {                                       \
        da_reserve((da), (da)->count + 1); \
        (da)->items[(da)->count++] = (item);   \
    } while (0)

#define da_remove(da, i)               \
    do {                                             \
        size_t jj = (i);                              \
        assert(jj < (da)->count);                 \
        (da)->items[jj] = (da)->items[--(da)->count]; \
    } while(0)

typedef struct {
    int id;
    int *items;
    size_t count;
    size_t capacity;
} Set;

typedef struct {
    Set *items;
    size_t count;
    size_t capacity;
} Matrix;

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Cover;

void delete_set_by_id(Matrix* sets, int id)
{
    for (int i=0; i<sets->count; i++) {
        if (sets->items[i].id == id) {
            da_remove(sets, i);
            return;
        }
    }
}

void print_sets(Matrix sets, Cover cover)
{
    for (int i=0; i<sets.count; i++) {
        printf("%02d: ", sets.items[i].id);
        for (int j=0; j<sets.items[i].count; j++) {
            printf("%d ", sets.items[i].items[j]);
        }

        for (int k=0; k<cover.count; k++) {
            if (sets.items[i].id == cover.items[k]) {
                printf(" +++");
                break;
            }
        }
        printf("\n");
    }
}

void exact_cover(Matrix *sets, Cover* cover)
{   
    if (sets->count == 0) return;

    // TODO: choose column with least 1s
    int column = 0, row = 0;
    for (; row<sets->items[row].count; row++) {
        if (sets->items[row].items[column] == 1) {
            da_append(cover, sets->items[row].id);
            break;
        }
    }

    Cover rows_to_delete = {0};
    Cover cols_to_delete = {0};
    for (int i=0; i<sets->items[row].count; i++) {
        if (sets->items[row].items[i] == 1) {
            for (int j=0; j<sets->count; j++) {
                if (sets->items[j].items[i] == 1) {
                    da_append(&rows_to_delete, sets->items[j].id);
                }
            }
            da_append(&cols_to_delete, i);
        }
    }

    for (int i=0; i<rows_to_delete.count; i++){
        delete_set_by_id(sets, rows_to_delete.items[i]);
    }

    for (int i=cols_to_delete.count-1; i>=0; i--) {
        for (int j=0; j<sets->count; j++) {
            da_remove(&(sets->items[j]), cols_to_delete.items[i]);
        }
    }

    exact_cover(sets, cover);
}

int main()
{
    int N = 9;
    Matrix sets = {0};
    for (int i=0; i<N*N; i++) {
        Set set = (Set) {0};
        set.id = i;
        da_append(&sets, set);
        for (int j=0; j<N; j++) {
            da_append(&sets.items[i], 0);
        }
    }

    
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            for (int k=0; k<N; k++) {
                if (i == j)
                    sets.items[k + j*N].items[i] = 1;
                else
                    sets.items[k + j*N].items[i] = 0;
            }
        }
    }

    Matrix initial_sets = {0};
    for (int i=0; i<sets.count; i++) {
        Set set = (Set) {0};
        set.id = i;
        da_append(&initial_sets, set);
        for (int j=0; j<sets.items[i].count; j++) {
            da_append(&initial_sets.items[i], sets.items[i].items[j]);
        }
    }

    Cover cover = {0};
    exact_cover(&sets, &cover);
    print_sets(initial_sets, cover);

    return 0;
}