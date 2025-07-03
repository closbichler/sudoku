#define EXACT_COVER_IMPLEMENTATION
#include "exact_cover.h"

typedef struct Node {
   struct Node *left, *right;
   struct Node *up, *down;
   struct Column *c;
   char val;
   int  set_id;
} Node;

typedef struct Column {
    struct Column *prev, *next;
    Node *head;
    int len;
    int id;
} Column;

void print_columns(Column *root)
{
    Column *c = root->next;
    int i = 0;
    printf("Col (len): \n");
    do {
        printf("%03d (%02d): ", c->id, c->len);
        if (c->head == NULL) {
            c = c->next;
            printf("\n");
            continue;
        }
        Node *n = c->head;
        do {
            if (n == c->head) printf(":head:=");
            printf("%d->", n->val);
            if (n->down == c->head) printf(":down-head:");
            if (n == c->head->up) printf(":head-up:");
            n = n->down;
            i++;
        } while (n != c->head && i < 20);
        printf("\n");
        c = c->next;
    } while (c != root && i < 20);
}

Column* matrix_to_linked_list(Matrix matrix)
{
    Column *root = malloc(sizeof(Column));
    root->next = root;
    root->prev = root;

    Column *c = root;
    for (int i=0; i<matrix.items[0].count; i++) {
        Column *new_col = malloc(sizeof(Column));
        new_col->len = 0;
        new_col->id = i;
        new_col->head = NULL;
        new_col->prev = c;
        new_col->next = root;
        root->prev = new_col;
        c->next = new_col;
        c = new_col;
    }
    
    for (int i=0; i<matrix.count; i++) {
        Column *c = root;
        Node *last_node = NULL;
        Node *first_node = NULL;
        for (int j=0; j<matrix.items[i].count; j++) {
            c = c->next;
            int val = matrix.items[i].items[j];
            if (val == 0) continue;

            Node *new_node = malloc(sizeof(Node));
            new_node->val = val;
            new_node->set_id = i;
            new_node->c = c;
            if (last_node == NULL) {
                last_node = new_node;
                first_node = new_node;
            }
            last_node->right = new_node;
            new_node->left = last_node;
            last_node = new_node;
            
            if (c->head == NULL) {
                c->head = new_node;
                c->head->up = c->head;
                c->head->down = c->head;
            } else {
                new_node->down = c->head;
                new_node->up = c->head->up;
                c->head->up->down = new_node;
                c->head->up = new_node;
            }
            c->len++;
        }
        last_node->right = first_node;
        first_node->left = last_node;
     }

    return root;
}

void remove_element_from_col(Node *n)
{
    if (n == n->c->head) {
        if (n->down == n) {
            n->c->head = NULL;
        } else {
            n->c->head = n->down;
        }
    }
    n->up->down = n->down;
    n->down->up = n->up;
    n->c->len--;
}

void unremove_element_from_col(Node *n)
{
    if (n->c->head == NULL) {
        n->c->head = n;
    } else if (n->up == n->c->head) {
        n->c->head->down = n;
    }
    n->up->down = n;
    n->down->up = n;
    n->c->len++;
}

void cover_column(Column *c)
{
    printf("cover column %d\n", c->id); 
    c->prev->next = c->next;
    c->next->prev = c->prev;
    Node *n = c->head;
    do {
        printf("remove row %d\n", n->set_id);
        Node *neighbor = n->right;
        while (neighbor != n) {
            remove_element_from_col(neighbor);
            neighbor = neighbor->right;
        }
        n = n->down;
    } while (n != c->head);
}

void uncover_column(Column *c) {
    c->prev->next = c;
    c->next->prev = c;
    Node *n = c->head;
    do {
        Node *neighbor = n->right;
        while (neighbor != n) {
            unremove_element_from_col(neighbor);
            neighbor = neighbor->right;
        }
        n = n->up;
    } while (n != c->head);
}

bool exact_cover_dlx(Column *root, Cover *cover)
{
    if (root->next == root) return true;

    // choose column with least 1s
    Column *c_iter = root->next;
    Column *c = c_iter;
    int min_len = c->len;
    while (c_iter != root) {
        if (c_iter->len < min_len) {
            c = c_iter;
            min_len = c->len;
        }
        c_iter = c_iter->next;
    }

    printf("select col %d with %d ones\n", c->id, min_len);
    
    if (min_len <= 0) return false;
    
    Node *n = c->head;
    do {
        print_columns(root);
        
        // cover row
        da_append(cover, n->set_id);
        Node *neighbor = n;
        do {
            cover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        if (exact_cover_dlx(root, cover))
            return true;

        // uncover row
        da_remove(cover, cover->count-1);
        neighbor = n;
        do {
            uncover_column(neighbor->c);
            neighbor = neighbor->right;
        } while (neighbor != n);

        n = n->down;
    } while (n != c->head);

    printf("wrong!!\n");
    return false;
}

void exact_cover_example_dlx()
{
    Matrix sets = {0};
    for (int i = 0; i < 6; i++) {
        Set set = (Set){0};
        set.id = i;
        set.count = 7;
        set.capacity = 7;
        da_append(&sets, set);
    }

    sets.items[0].items = (int[]) { 1,0,0,1,0,0,1 };
    sets.items[1].items = (int[]) { 1,0,0,1,0,0,0 };
    sets.items[2].items = (int[]) { 0,0,0,1,1,0,1 };
    sets.items[3].items = (int[]) { 0,0,1,0,1,1,0 };
    sets.items[4].items = (int[]) { 0,1,1,0,0,1,1 };
    sets.items[5].items = (int[]) { 0,1,0,0,0,0,1 };

    Column *root = matrix_to_linked_list(sets);
    Cover cover = { 0 };
    if (!exact_cover_dlx(root, &cover)) printf("no solution\n");
    exact_cover_print_sets(sets, cover);
}

int main()
{
    exact_cover_example_dlx();
    return 1;

    Matrix matrix = exact_cover_create_sudoku_constraint_sets(9, 3);
    Column *root = matrix_to_linked_list(matrix);
    Cover cover = {0};
    exact_cover_dlx(root, &cover);
    
    // print_columns(root);
    // exact_cover_print_sets(matrix, cover);

    return 0;
}