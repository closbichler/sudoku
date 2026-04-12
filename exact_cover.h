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
typedef unsigned char uint8_t;
typedef unsigned long int ulong;
#endif // size_t

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
    long hash;
    ulong solutions;
    SetCoverHashEntry* next;
};

typedef struct {
    SetCoverHashEntry* items;
    size_t capacity;
    size_t count;
} SetCoverHashTable;

/* TODO: comment */
ulong exact_solve_constriants(uint8_t **constraints, int num_rows, int num_cols, SetCover *cover, int find_first_solution_only, int max_solutions);
ulong exact_solve(DLXColumn *root, SetCover *cover, int find_first_solution_only, int max_solutions);

#endif // EXACT_COVER_H


#ifdef EXACT_COVER_IMPLEMENTATION
#ifndef EXACT_COVER_IMPLEMENTED
#define EXACT_COVER_IMPLEMENTED

#ifndef NULL
#define NULL ((void*)0)
#endif // NULL

#ifndef size_t
typedef __SIZE_TYPE__ size_t;
typedef unsigned char uint8_t;
typedef unsigned long int ulong;
#endif // size_t

// Link these with some implementation
void *malloc(size_t size);
void free(void *p);


// -- DA Macros from www.github.com/tsoding/nob.h --

#define da_reserve(da, expected_capacity)                                              \
    do                                                                                 \
    {                                                                                  \
        if ((expected_capacity) > (da)->capacity) {                                    \
            if ((da)->capacity == 0) {                                                 \
                (da)->capacity = 256;                                                  \
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


// Global variables for dynamic programming table and flag to enable/disable it
// TODO: rework, cause it slows down than speeds up
SetCoverHashTable table;
int ff_dynamic_programming = 1;

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

long exact_hash_setcover(SetCover setcover)
{
    long hash = 1;
    for (int i=0; i<setcover.count; i++) {
        hash ^= setcover.items[i] * 4242829781;//0x9e3779b1;
    }
    return hash;
}

int exact_lookup_setcover_solutions(SetCover setcover)
{
    long hash = exact_hash_setcover(setcover);
    size_t index = hash % table.capacity;

    SetCoverHashEntry entry = table.items[index];
    if (entry.solutions != -1) {
        if (entry.hash == hash) {
            return (int) entry.solutions;
        } else {
            SetCoverHashEntry* e = entry.next;
            while (e != NULL) {
                if (e->hash == hash) 
                    return e->solutions;
                e = e->next;
            }
        }
    }
    return -1;
}

void exact_store_setcover_solutions(SetCover setcover, ulong solutions)
{
    long hash = exact_hash_setcover(setcover);
    size_t index = hash % table.capacity;

    if (table.items[index].solutions == -1) {
        table.items[index] = (SetCoverHashEntry) {
            .hash = hash,
            .solutions = solutions,
        };
    } else {
        SetCoverHashEntry* e = &table.items[index];
        while (e->next != NULL) {
            e = e->next;
        }
        e->next = (SetCoverHashEntry*) malloc(sizeof(SetCoverHashEntry));
        e->next->hash = hash;
        e->next->solutions = solutions;
        e->next->next = NULL;
    }
}

void exact_create_setcover_hashtable(int size) {
    table = (SetCoverHashTable) {0};
    da_reserve(&table, size);
    table.count = table.capacity;

    for (int i=0; i<table.count; i++) {
        // NULL-element
        table.items[i] = (SetCoverHashEntry) {
            .hash = -1,
            .solutions = -1,
            .next = NULL,
        };
    }
}

ulong exact_solve(DLXColumn *root, SetCover *cover, int find_first_solution_only, int max_solutions)
{
    if (root->next == root) return 1;

    ulong solutions = 0;

    // choose column with least elements
    DLXColumn *start_column = exact_get_shortest_column(root);
    DLXNode *n = start_column->head;
    if (n == NULL) return solutions;

    do {
        da_append(cover, n->set_id);

        if (ff_dynamic_programming && exact_lookup_setcover_solutions(*cover) != -1) {
            da_remove(cover, cover->count-1);   
            n = n->down;
            continue;
        }

        // cover row
        DLXNode *neighbor = n;
        do {
            exact_cover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        // test solution
        ulong sub_solutions = exact_solve(root, cover, find_first_solution_only, max_solutions);
        solutions += sub_solutions;
        if (sub_solutions > 0 && find_first_solution_only)
            return 1;

        if (max_solutions != -1 && solutions > max_solutions)
            return solutions;

        if (ff_dynamic_programming) {
            exact_store_setcover_solutions(*cover, sub_solutions);
        }
        
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

#endif // EXACT_COVER_IMPLEMENTED
#endif // EXACT_COVER_IMPLEMENTATION