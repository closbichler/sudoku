/*----------------------
 | EXACT_COVER - Exact Cover implementation in C
 |
 | Author: Clemens Losbichler
 | Dependencies: any malloc implementation
 ----------------------*/

#ifndef EXACT_COVER_H
#define EXACT_COVER_H

#ifndef size_t
typedef __SIZE_TYPE__ size_t;
#endif // size_t

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif // uint8_t

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} SetCover;

typedef struct {
    int id;
    int *items;
    size_t count;
    size_t capacity;
} Set;

typedef struct DLXNode {
   struct DLXNode *left, *right;
   struct DLXNode *up, *down;
   struct DLXColumn *c;
   uint8_t val;
   int  set_id;
} DLXNode;

typedef struct DLXColumn {
    struct DLXColumn *prev, *next;
    DLXNode *head;
    int len;
    int id;
} DLXColumn;

typedef struct SetCoverHashEntry SetCoverHashEntry;
struct SetCoverHashEntry {
    unsigned long hash;
    unsigned long solutions;
    int *key_items;
    size_t key_count;
    SetCoverHashEntry* next;
};

#define HASH_CONST 0x9e3779b9
#define HASHENTRY_NOT_FOUND ((unsigned long)(-1L))

typedef struct {
    SetCoverHashEntry* items;
    size_t capacity;
    size_t count;
    
    unsigned long lookup_count;
    unsigned long hit_count;
} SetCoverHashTable;


/* TODO: comment */
unsigned long exact_solve_constriants(uint8_t **constraints, int num_rows, int num_cols, SetCover *cover, int find_first_solution_only, unsigned long max_solutions);
unsigned long exact_solve(DLXColumn *root, SetCover *cover, int find_first_solution_only, unsigned long max_solutions);

#endif // EXACT_COVER_H


#ifdef EXACT_COVER_IMPLEMENTATION
#ifndef EXACT_COVER_IMPLEMENTED
#define EXACT_COVER_IMPLEMENTED

#ifdef EXACT_COVER_DEBUG
#include <stdio.h>
#endif // EXACT_COVER_DEBUG

#ifndef NULL
#define NULL ((void*)0)
#endif // NULL

// Link these with some implementation
void *malloc(size_t size);
void free(void *p);

// -- DA Macros from www.github.com/tsoding/nob.h --

#define da_reserve(da, expected_capacity)                                              \
    do                                                                                 \
    {                                                                                  \
        if ((expected_capacity) > (da)->capacity) {                                    \
            if ((da)->capacity == 0) {                                                 \
                (da)->capacity = (expected_capacity);                                                  \
            }                                                                          \
            while ((expected_capacity) > (da)->capacity) {                             \
                (da)->capacity *= 2;                                                   \
            }                                                                          \
            void* new_items = malloc((da)->capacity * sizeof(*(da)->items));           \
            if (new_items == NULL) {                                                   \
                /* exit (-1004) */                                                     \
            }                                                                          \
            if ((da)->items != NULL)                                                   \
            {                                                                          \
                uint8_t *dst = (uint8_t *)new_items;                                         \
                uint8_t *src = (uint8_t *)(da)->items;                                       \
                size_t bytes = (da)->count * sizeof(*(da)->items);                     \
                for (size_t i = 0; i < bytes; ++i) dst[i] = src[i];                    \
                free((da)->items);                                                     \
            }                                                                          \
            (da)->items = new_items;                                                   \
        }                                                                              \
    } while (0)

#define da_reserve_1(da, expected_capacity)                                              \
    do                                                                                 \
    {                                                                                  \
        if ((expected_capacity) > (da)->capacity) {                                    \
            if ((da)->capacity == 0) {                                                 \
                (da)->capacity = 256;                                                  \
            }                                                                          \
            while ((expected_capacity) > (da)->capacity) {                             \
                (da)->capacity *= 2;                                                   \
            }                                                                          \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof(*(da)->items)); \
        }                                                                              \
    } while (0)

#define da_append(da, item)                  \
    do                                       \
    {                                        \
        da_reserve((da), (da)->count + 1);   \
        (da)->items[(da)->count++] = (item); \
    } while (0)

#define da_remove(da, i)                              \
    do                                                \
    {                                                 \
        size_t jj = (i);                              \
        if (jj >= (da)->count) {                      \
            /* exit(-1003);*/                         \
            break;                                    \
        }                                             \
        (da)->count--;                                \
        for (int jjj=jj; jjj<(da)->count; jjj++) {    \
            (da)->items[jjj] = (da)->items[jjj+1];    \
        }                                             \
    } while (0)

DLXColumn* exact_constraints_to_dlx(uint8_t **constraints, int num_rows, int num_cols)
{
    DLXColumn *root = malloc(sizeof(DLXColumn));
    root->next = root;
    root->prev = root;

    DLXColumn *c = root;
    for (int i=0; i<num_cols; i++) {
        DLXColumn *new_col = malloc(sizeof(DLXColumn));
        new_col->len = 0;
        new_col->id = i;
        new_col->head = NULL;
        new_col->prev = c;
        new_col->next = root;
        root->prev = new_col;
        c->next = new_col;
        c = new_col;
    }
    
    for (int i=0; i<num_rows; i++) {
        DLXColumn *c = root;
        DLXNode *last_DLXnode = NULL;
        DLXNode *first_DLXnode = NULL;
        for (int j=0; j<num_cols; j++) {            
            c = c->next;
            int val = constraints[i][j];
            if (val == 0) continue;

            DLXNode *new_DLXnode = malloc(sizeof(DLXNode));
            new_DLXnode->val = val;
            new_DLXnode->set_id = i;
            new_DLXnode->c = c;
            if (last_DLXnode == NULL) {
                last_DLXnode = new_DLXnode;
                first_DLXnode = new_DLXnode;
            }
            last_DLXnode->right = new_DLXnode;
            new_DLXnode->left = last_DLXnode;
            last_DLXnode = new_DLXnode;
            
            if (c->head == NULL) {
                c->head = new_DLXnode;
                c->head->up = c->head;
                c->head->down = c->head;
            } else {
                new_DLXnode->down = c->head;
                new_DLXnode->up = c->head->up;
                c->head->up->down = new_DLXnode;
                c->head->up = new_DLXnode;
            }
            c->len++;
        }
        if (last_DLXnode != NULL && first_DLXnode != NULL) {
            last_DLXnode->right = first_DLXnode;
            first_DLXnode->left = last_DLXnode;
        }
    }

    return root;
}

void exact_remove_node(DLXNode *n)
{
    if (n == n->c->head && n->down == n) {
        // column consists of only this node
        n->c->head = NULL;
        n->c->len--;
    } else if (n == n->c->head) {
        n->c->head = n->down;
        n->up->down = n->down;
        n->down->up = n->up;
        n->c->len--;
    } else {
        n->up->down = n->down;
        n->down->up = n->up;
        n->c->len--;
    }
}

void exact_unremove_node(DLXNode *n)
{
    if (n->c->head == NULL) {
        // column empty, add node as head
        n->c->head = n;
        n->up = n;
        n->down = n;
        n->c->len++;
    } else if (n->up->set_id > n->set_id) {
        // node should be new head
        n->c->head = n;
        n->up->down = n;
        n->down->up = n;
        n->c->len++;
    } else if (n->up->down == n && n->down->up == n) {
        // already in list, do nothing
    } else {
        n->up->down = n;
        n->down->up = n;
        n->c->len++;
    }
}

void exact_cover_column(DLXColumn *c)
{
    c->prev->next = c->next;
    c->next->prev = c->prev;

    DLXNode *n = c->head;
    if (n == NULL) return;
    do {
        DLXNode *neighbor = n->right;
        while (neighbor != n) {
            exact_remove_node(neighbor);
            neighbor = neighbor->right;
        }
        n = n->down;
    } while (n != c->head);
}

void exact_uncover_column(DLXColumn *c) 
{
    // Reinsert all nodes from each row in this column back into
    // their respective columns, iterating in reverse (up) order.
    
    if (c->head == NULL) {
        // column empty, just reattach
        c->prev->next = c;
        c->next->prev = c;
        return;    
    }

    DLXNode *n = c->head->up;
    if (n != NULL) {
        do {
            DLXNode *neighbor = n->left;
            while (neighbor != n) {
                exact_unremove_node(neighbor);
                neighbor = neighbor->left;
            }
            n = n->up;
        } while (n != c->head->up);
    }

    // Reattach this column into the header list.
    c->prev->next = c;
    c->next->prev = c;
}

DLXColumn* exact_get_shortest_column(DLXColumn *root)
{
    DLXColumn *c = root->next;
    DLXColumn *shortest_column = c;
    int min_len = c->len;
    while (c != root) {
        if (c->len < min_len) {
            shortest_column = c;
            min_len = c->len;
        }
        c = c->next;
    }
    return shortest_column;
}

void exact_delete_possibility(DLXColumn *root, int set_id)
{
    DLXColumn *c = root->next;
    while (c != root) {
        DLXNode *n = c->head;

        if (n == NULL) { 
            c = c->next; 
            continue; 
        }

        do {
            if (n->set_id == set_id) {
                DLXNode *neighbor = n;
                do {
                    exact_cover_column(neighbor->c);
                    neighbor = neighbor->right;
                } while (n != neighbor);

                return;
            }
            n = n->down;
        } while (n != c->head);

        c = c->next;
    }
}

unsigned long exact_hash_key_items(const int *items, size_t count)
{
    unsigned long hash = 1;
    for (size_t i = 0; i < count; i++) {
        hash ^= items[i] * HASH_CONST;
    }
    return hash;
}

int* exact_get_setcover_key(SetCover setcover) 
{
    if (setcover.count == 0) return NULL;

    int* key_items = malloc(setcover.count * sizeof(int));
    for (size_t i = 0; i < setcover.count; i++) {
        key_items[i] = setcover.items[i];
    }

    // Keep key canonical so different traversal orders can hit the same cache entry.
    for (size_t i = 1; i < setcover.count; i++) {
        int x = key_items[i];
        size_t j = i;
        while (j > 0 && key_items[j - 1] > x) {
            key_items[j] = key_items[j - 1];
            j--;
        }
        key_items[j] = x;
    }

    return key_items;
}

int exact_hashtable_entry_matches(SetCoverHashEntry entry, const int *key_items, size_t key_count, unsigned long hash)
{
    if (entry.hash != hash) return 0;
    if (entry.key_count != key_count) return 0;
    for (size_t i = 0; i < key_count; i++) {
        if (entry.key_items[i] != key_items[i]) return 0;
    }
    return 1;
}

unsigned long exact_lookup_setcover_solutions(SetCoverHashTable *hashtable, SetCover setcover)
{
    if (hashtable->capacity == 0) return HASHENTRY_NOT_FOUND;

    hashtable->lookup_count++;
    int *key_items = exact_get_setcover_key(setcover);
    unsigned long hash = exact_hash_key_items(key_items, setcover.count);
    size_t index = hash % hashtable->capacity;

    SetCoverHashEntry entry = hashtable->items[index];
    if (entry.solutions != HASHENTRY_NOT_FOUND) {
        if (exact_hashtable_entry_matches(entry, key_items, setcover.count, hash)) {
            hashtable->hit_count++;
            free(key_items);
            return entry.solutions;
        }

        SetCoverHashEntry* e = entry.next;
        while (e != NULL) {
            if (exact_hashtable_entry_matches(*e, key_items, setcover.count, hash)) {
                hashtable->hit_count++;
                free(key_items);
                return e->solutions;
            }
            e = e->next;
        }
    }

    free(key_items);
    return HASHENTRY_NOT_FOUND;
}

void exact_store_setcover_solutions(SetCoverHashTable *hashtable, SetCover setcover, unsigned long solutions)
{
    if (hashtable->capacity == 0) return;
    
    int *key_items = exact_get_setcover_key(setcover);
    unsigned long hash = exact_hash_key_items(key_items, setcover.count);
    size_t index = hash % hashtable->capacity;
    if (hashtable->items[index].solutions == HASHENTRY_NOT_FOUND) {
        hashtable->items[index].hash = hash;
        hashtable->items[index].solutions = solutions;
        hashtable->items[index].key_count = setcover.count;
        hashtable->items[index].key_items = key_items;
    } else {
        SetCoverHashEntry* e = &hashtable->items[index];
        int list_size = 0;
        while (e->next != NULL) {
            e = e->next;
            list_size++;
            if (list_size > hashtable->capacity) return;
        }
        e->next = (SetCoverHashEntry*) malloc(sizeof(SetCoverHashEntry));
        e->next->hash = hash;
        e->next->solutions = solutions;
        e->next->next = NULL;
        e->next->key_count = setcover.count;
        e->next->key_items = key_items;
    }
}

SetCoverHashTable exact_create_setcover_hashtable(int size) {
    SetCoverHashTable hashtable = (SetCoverHashTable) {0};
    da_reserve(&hashtable, size);
    hashtable.count = hashtable.capacity;

    for (int i=0; i<hashtable.count; i++) {
        // NULL-element
        hashtable.items[i] = (SetCoverHashEntry) {
            .hash      = 1,
            .solutions = HASHENTRY_NOT_FOUND,
            .next      = NULL,
            .key_items = NULL,
            .key_count = 0,
        };
    }

    return hashtable;
}

unsigned long exact_solve_with_lookup(DLXColumn *root, SetCover *cover, SetCoverHashTable *hashtable, int find_first_solution_only, unsigned long max_solutions)
{
    if (root->next == root) return 1;

    unsigned long solutions = 0;

    // choose column with least elements
    DLXColumn *start_column = exact_get_shortest_column(root);
    DLXNode *n = start_column->head;
    if (n == NULL) return solutions;

    do {
        da_append(cover, n->set_id);

        unsigned long cached_solutions = exact_lookup_setcover_solutions(hashtable, *cover);
        if (cached_solutions != HASHENTRY_NOT_FOUND) {
            solutions += cached_solutions;
            da_remove(cover, cover->count-1);
            n = n->down;

            if (cached_solutions > 0 && find_first_solution_only)
                return 1;
            continue;
        }

        // cover row
        DLXNode *neighbor = n;
        do {
            exact_cover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        // test solution
        unsigned long sub_solutions = exact_solve_with_lookup(root, cover, hashtable, find_first_solution_only, max_solutions);
        solutions += sub_solutions;
        if (sub_solutions > 0 && find_first_solution_only)
            return 1;

        if (max_solutions != -1 && solutions > max_solutions)
            return solutions;

        exact_store_setcover_solutions(hashtable, *cover, sub_solutions);
        
        // uncover row
        da_remove(cover, cover->count-1);
        neighbor = n->left;
        do {
            exact_uncover_column(neighbor->c);
            neighbor = neighbor->left;
        } while (neighbor != n->left);

        n = n->down;
    } while (n != start_column->head);
    
    return solutions > 0 && find_first_solution_only ? 1 : solutions;
}

unsigned long exact_solve(DLXColumn *root, SetCover *cover, int find_first_solution_only, unsigned long max_solutions)
{
    SetCoverHashTable hashtable = exact_create_setcover_hashtable(9*9*9);
    
    unsigned long solutions = exact_solve_with_lookup(root, cover, &hashtable, find_first_solution_only, max_solutions);

    #ifdef EXACT_COVER_DEBUG
    fprintf(stdout, "==Hashtable Stats==\n");
    fprintf(stdout, "capacity %zu\n", hashtable.capacity);
    fprintf(stdout, "lookups   %zu\n", hashtable.lookup_count);
    fprintf(stdout, "hits      %zu\n", hashtable.hit_count);
    fprintf(stdout, "hit-rate  %.4f\n", (double)(hashtable.hit_count) / (hashtable.lookup_count == 0 ? 1 : hashtable.lookup_count));
    #endif // EXACT_COVER_DEBUG

    return solutions;
}

unsigned long exact_solve_without_dp(DLXColumn *root, SetCover *cover, int find_first_solution_only, unsigned long max_solutions)
{
    SetCoverHashTable hashtable = { .capacity = 0 };
    return exact_solve_with_lookup(root, cover, &hashtable, find_first_solution_only, max_solutions);
}

unsigned long exact_solve_constraints(uint8_t **constraints, int num_rows, int num_cols, SetCover *cover, int find_first_solution_only, unsigned long max_solutions)
{
    DLXColumn *root = exact_constraints_to_dlx(constraints, num_rows, num_cols);
    return exact_solve(root, cover, find_first_solution_only, max_solutions);
}

unsigned long exact_solve_constraints_without_dp(uint8_t **constraints, int num_rows, int num_cols, SetCover *cover, int find_first_solution_only, unsigned long max_solutions)
{
    DLXColumn *root = exact_constraints_to_dlx(constraints, num_rows, num_cols);
    return exact_solve_without_dp(root, cover, find_first_solution_only, max_solutions);
}

#endif // EXACT_COVER_IMPLEMENTED
#endif // EXACT_COVER_IMPLEMENTATION