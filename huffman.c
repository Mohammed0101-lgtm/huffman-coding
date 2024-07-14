#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

struct node {
    char symbol;
    unsigned weight;
    struct node *left;
    struct node *right;
};

struct entry {
    char *symbol;
    unsigned weight;
    struct entry* next;
};

struct map {
    struct entry **buckets;
    size_t capacity;
    size_t size;
};

unsigned long long hash(const char *str) {
    unsigned long long h = 5381;
    int c;
    while ((c = *str++)) {
        h = ((h << 5) + h) + c;
    }
    return h;
}

struct map *create_map(size_t capacity) {
    struct map *m = (struct map*)malloc(sizeof(struct map));
    if (m == NULL) return NULL;

    m->capacity = capacity;
    m->size = 0;
    m->buckets = (struct entry**)calloc(capacity, sizeof(struct entry*));
    if (!m->buckets) {
        free(m);
        return NULL;
    }

    return m;
}

int get(struct map *m, const char *key) {
    int index = hash(key) % m->capacity;
    struct entry *e = m->buckets[index];

    while (e != NULL) {
        if (strcmp(key, e->symbol) == 0) {
            return e->weight;
        }
        e = e->next;
    }

    return 0;
}

void put(struct map *m, const char *key, int value) {
    int index = hash(key) % m->capacity;
    struct entry *e = m->buckets[index];

    while (e != NULL) {
        if (strcmp(e->symbol, key) == 0) {
            e->weight = value;
            return;
        }
        e = e->next;
    }

    struct entry *new = (struct entry *)malloc(sizeof(struct entry));
    if (new == NULL) return;

    new->weight = value;
    new->symbol = strdup(key);
    if (!new->symbol) {
        free(new);
        return;
    }

    new->next = m->buckets[index];
    m->buckets[index] = new;
    m->size++;
}

void free_map(struct map *m) {
    for (int i = 0; i < m->capacity; i++) {
        struct entry *e = m->buckets[i];
        while (e) {
            struct entry *next = e->next;
            free(e->symbol);
            free(e);
            e = next;
        }
    }
    free(m->buckets);
    free(m);
}

struct priority_queue {
    struct entry *top;
    size_t size;
    size_t capacity;
};

struct priority_queue *create_pq(size_t capacity) {
    size_t max_cap = 1024;
    if (capacity > max_cap) capacity = max_cap;

    struct priority_queue *p = (struct priority_queue*)malloc(sizeof(struct priority_queue));
    if (p == NULL) {
        fprintf(stderr, "Failed to allocate memory for priority queue!\n");
        return NULL;
    }

    p->capacity = capacity;
    p->top = NULL;
    p->size = 0;

    return p;
}

void pop(struct priority_queue **stack) {
    if (stack == NULL || *stack == NULL || (*stack)->top == NULL) return;

    struct entry *new_top = (*stack)->top->next;
    free((*stack)->top);
    (*stack)->top = new_top;
    (*stack)->size--;
}

void push(struct priority_queue **stack, struct entry *new) {
    if (stack == NULL || *stack == NULL) return;

    if ((*stack)->top == NULL || new->weight > (*stack)->top->weight) {
        new->next = (*stack)->top;
        (*stack)->top = new;
        (*stack)->size++;
        return;
    }

    struct entry *e = (*stack)->top;
    while (e->next != NULL && new->weight <= e->next->weight) {
        e = e->next;
    }

    new->next = e->next;
    e->next = new;
    (*stack)->size++;
}

#define MAX_CAP 256

struct node* build_tree(struct map *freq) {
    struct priority_queue *stack = create_pq(MAX_CAP);

    for (size_t i = 0; i < freq->capacity; i++) {
        struct entry *e = freq->buckets[i];
        while (e != NULL) {
            struct entry *new = (struct entry*)malloc(sizeof(struct entry));
            if (new == NULL) return NULL;

            new->symbol = strdup(e->symbol);
            new->weight = e->weight;
            new->next = NULL;
            push(&stack, new);
            e = e->next;
        }
    }

    while (stack->size > 1) {
        struct node *left = (struct node *)malloc(sizeof(struct node));
        if (left == NULL) return NULL;
        left->symbol = stack->top->symbol[0];
        left->weight = stack->top->weight;
        left->left = NULL;
        left->right = NULL;
        pop(&stack);

        struct node *right = (struct node *)malloc(sizeof(struct node));
        if (right == NULL) return NULL;
        right->symbol = stack->top->symbol[0];
        right->weight = stack->top->weight;
        right->left = NULL;
        right->right = NULL;
        pop(&stack);

        struct node *parent = (struct node *)malloc(sizeof(struct node));
        if (parent == NULL) return NULL;
        parent->left = left;
        parent->right = right;
        parent->weight = left->weight + right->weight;
        parent->symbol = '$';

        struct entry *new_entry = (struct entry *)malloc(sizeof(struct entry));
        if (new_entry == NULL) return NULL;
        new_entry->symbol = strdup("$");
        new_entry->weight = parent->weight;
        new_entry->next = NULL;
        push(&stack, new_entry);
    }

    struct node *root = (struct node *)malloc(sizeof(struct node));
    if (root == NULL) return NULL;
    root->symbol = stack->top->symbol[0];
    root->weight = stack->top->weight;
    root->left = NULL;
    root->right = NULL;
    return root;
}

void free_tree(struct node *root) {
    if (root == NULL) return;

    free_tree(root->left);
    free_tree(root->right);

    free(root);
}

bool isleaf(struct node *n) {
    return !(n->left) && !(n->right);
}

const char *compress(const char *input) {
    size_t len = strlen(input);
    struct map *m = create_map(MAX_CAP);

    for (size_t i = 0; i < len; i++) {
        char key[2] = { input[i], '\0' };
        int val = get(m, key);
        val++;
        put(m, key, val);
    }

    struct node *root = build_tree(m);
    if (root == NULL) {
        free_map(m);
        fprintf(stderr, "Failed to construct Huffman tree!\n");
        return NULL;
    }

    char *codes = (char*)malloc(MAX_CAP * sizeof(char));
    if (codes == NULL) {
        free_map(m);
        free_tree(root);
        fprintf(stderr, "Memory allocation failed!\n");
        return NULL;
    }

    *codes = '\0';
    // TODO: store the Huffman codes from the tree into codes

    free_map(m);
    free_tree(root);
    return codes;
}

int main() {
    char path[PATH_MAX];
    printf("Enter file path: ");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        fprintf(stderr, "Failed to register input!\n");
        return -1;
    }
    path[strcspn(path, "\n")] = '\0';

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file!\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char*)malloc(file_size * sizeof(char) + 1);
    if (buffer == NULL) {
        fclose(fp);
        fprintf(stderr, "Memory allocation failed!\n");
        return -1;
    }

    if (fread(buffer, sizeof(char), file_size, fp) != file_size) {
        fclose(fp);
        free(buffer);
        fprintf(stderr, "Failed to load file data!\n");
        return -1;
    }

    buffer[file_size] = '\0';
    fclose(fp);

    const char *codes = compress(buffer);
    if (codes == NULL) {
        fprintf(stderr, "Failed to compress input!\n");
        free(buffer);
        return -1;
    }

    free(buffer);

    printf("%s\n", codes);

    free((void*)codes);
    return 0;
}