#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h> 

// binary tree node
struct node {
    char symbol;        // character in the input
    unsigned weight;    // frequency of the character
    
    struct node *left;  // left child node
    struct node *right; // right child node
};

// entry of the hash map
struct entry {
    char *symbol;       // key
    unsigned weight;    // value
    
    struct entry* next; // next entry
};

// hash map 
struct map {
    struct entry **buckets; // buckets of entries
    
    size_t capacity;        // maximum capacity
    size_t size;            // current size
};

// hashing the key to get index
unsigned long long hash(const char *str) {
    unsigned long long h = 5381;
    int c;

    while ((c = *str++)) 
        h = ((h << 5) + h) + c;
    
    return h;
}

// initialize map
struct map *create_map(size_t capacity) {
    // allocate memory for the map structure
    struct map *m = (struct map*)malloc(sizeof(struct map));
    
    if (!m) 
        return NULL;

    m->capacity = capacity; // set capacity
    m->size     = 0; // set initial size
    
    // co_allocate memory for the buckets of entry
    m->buckets = (struct entry**)calloc(capacity, sizeof(struct entry*));
    
    if (!(m->buckets)) {
        free(m);
        return NULL;
    }

    return m;
}

// retrieve value of an input key
int get(struct map *m, const char *key) {
    // hash the key to get the index
    int index       = hash(key) % m->capacity;
    // create an entry at the bucket
    struct entry *e = m->buckets[index];

    // go through the bucket 
    while (e) {
        // if the key is found then return it's value
        if (strcmp(key, e->symbol) == 0) 
            return e->weight; 
        
        e = e->next;
    }

    return 0; // return 0 if entry is not found
}

// add new entry to map
void put(struct map *m, const char *key, int value) {
    // get the appropriate index of the key
    int index       = hash(key) % m->capacity;
    struct entry *e = m->buckets[index];

    // check if the key already exists
    while (e != NULL) {
        // if the key is found then update the value
        if (strcmp(e->symbol, key) == 0) {
            e->weight = value;
            return;
        }
    
        e = e->next;
    }

    // if key is not found then create new entry
    struct entry *new = (struct entry *)malloc(sizeof(struct entry));
    
    if (!new) 
        return;

    // initialize entry
    new->weight = value; 
    new->symbol = strdup(key);
    
    if (!(new->symbol)) {
        free(new);
        return;
    }

    // place entry at the index
    new->next         = m->buckets[index];
    m->buckets[index] = new;
    
    m->size++; // increment map size
}

// free the map structure
void free_map(struct map *m) {
    for (int i = 0; i < m->capacity; i++) {
        // free a bucket at once
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

// create a queue structure
struct priority_queue {
    struct entry *top; // the maximum of the stack
    
    size_t size;
    size_t capacity;
};

// create new priority queue instance
struct priority_queue *create_pq(size_t capacity) {
    size_t max_cap = 1024; // maximum capacity
    
    if (capacity > max_cap) 
        capacity = max_cap; // update the max_cap 

    // allocate memory for the structure
    struct priority_queue *p = (struct priority_queue*)malloc(sizeof(struct priority_queue));
    
    if (!p) {
        fprintf(stderr, 
                    "Failed to allocate memory for priority queue!\n");
        return NULL;
    }

    // initialize structure
    p->capacity = capacity;
    p->top      = NULL;
    p->size     = 0;

    return p;
}

// remove the maximum element of the stack
void pop(struct priority_queue **stack) {
    // if the stack is empty
    if (!stack || !(*stack) || !(*stack)->top) 
        return;

    // new top of the stack is top->next , which is the second maximum 
    struct entry *new_top = (*stack)->top->next; 
    
    // free the current top
    free((*stack)->top);
    
    // set the top to the second maximum
    (*stack)->top = new_top;
    
    // decrement the maximum
    (*stack)->size--;
}

// add an element to the stack
void push(struct priority_queue **stack, struct entry *new) {
    // if the stack is not initialized 
    // to prevent bad access
    if (!stack || !(*stack))
        return;

    // if the stack is empty then add the new entry 
    if ((*stack)->top == NULL || new->weight > (*stack)->top->weight) {
        new->next     = (*stack)->top;
        (*stack)->top = new;
        (*stack)->size++;
        
        return;
    }

    // tranverse the stack to get the position for insertion
    struct entry *e = (*stack)->top;

    while (e->next != NULL && new->weight <= e->next->weight) 
        e = e->next;

    // if the value of the new element is less than the current element
    new->next = e->next;
    e->next   = new;
    
    (*stack)->size++; // increment stack size
}

#define MAX_CAP 256 // maximum capacity

// building the huffman tree of the input
struct node* build_tree(struct map *freq) {
    // initialize stack
    struct priority_queue *stack = create_pq(MAX_CAP);

    // traversing the frequency map
    for (size_t i = 0; i < freq->capacity; i++) {
        // go through each bucket at once
        struct entry *e = freq->buckets[i];
        
        while (e) {
            // initialize new element of the stack
            struct entry *new = (struct entry*)malloc(sizeof(struct entry));

            if (!new) 
                return NULL;

            new->symbol = strdup(e->symbol); // set symbol
            new->weight = e->weight; // set frequency
            new->next   = NULL; // point next to null
            
            push(&stack, new); // push element to the stack
            
            e = e->next;
        }
    }

    // while the stack has more than one element
    // if the stack has only one element left means
    // that we have the root node on top
    while (stack->size > 1) {
        // initialize left child node
        struct node *left = (struct node *)malloc(sizeof(struct node));
        
        if (!left) 
            return NULL;
        
        left->symbol = stack->top->symbol[0]; 
        left->weight = stack->top->weight;
        left->left   = NULL;
        left->right  = NULL;
        
        pop(&stack);

        // initialize the right child node
        struct node *right = (struct node *)malloc(sizeof(struct node));
        
        if (!right) 
            return NULL;
        
        right->symbol = stack->top->symbol[0];
        right->weight = stack->top->weight;
        right->left   = NULL;
        right->right  = NULL;
        
        pop(&stack);

        // join the right and child nodes by a parent node
        struct node *parent = (struct node *)malloc(sizeof(struct node));
        
        if (!parent) 
            return NULL;
        
        parent->left   = left;
        parent->right  = right;
        parent->weight = left->weight + right->weight; // parent node represents both children weights
        parent->symbol = '$'; // no need for the symbol in this node

        // push the parent node to the stack
        struct entry *new_entry = (struct entry *)malloc(sizeof(struct entry));
        
        if (!new_entry) 
            return NULL;
        
        new_entry->symbol = strdup("$");
        new_entry->weight = parent->weight;
        new_entry->next   = NULL;
        
        push(&stack, new_entry);
    }

    // return the root node
    struct node *root = (struct node *)malloc(sizeof(struct node));
   
    if (!root) 
        return NULL;
    
    root->symbol = stack->top->symbol[0];
    root->weight = stack->top->weight;
    root->left   = NULL;
    root->right  = NULL;
    
    return root;
}

// free the tree structure 
void free_tree(struct node *root) {
    if (!root) 
        return;

    free_tree(root->left);
    free_tree(root->right);

    free(root);
}

// is the node a leaf node
bool isleaf(struct node *n) 
{ 
    return !(n->left) && !(n->right); 
}

// data compression function
const char *compress(const char *input) {
    // get the size of the input
    size_t len    = strlen(input);
    struct map *m = create_map(MAX_CAP);

    if (!m) 
        return NULL;

    // get the frequency / weight of each char
    for (size_t i = 0; i < len; i++) {
        char key[2] = { input[i] , '\0' };
        int val     = get(m, key);
        
        val++;
        put(m, key, val);
    }

    // create the tree structure
    struct node *root = build_tree(m);
    if (!root) {
        free_map(m);
        fprintf(stderr, 
                    "Failed to construct Huffman tree!\n");
        
        return NULL;
    }

    // string to store the codes for storage
    char *codes = (char*)malloc(MAX_CAP * sizeof(char));
    if (!codes) {
        free_map(m);
        free_tree(root);
        fprintf(stderr, 
                    "Memory allocation failed!\n");
        
        return NULL;
    }

    *codes = '\0'; // initialize the string
    // TODO: store the Huffman codes from the tree into codes

    free_map(m);
    free_tree(root);
    
    return codes; // return the codes
}

int main(void) {
    // get the filepath
    char path[PATH_MAX];
    printf("Enter file path: ");
    
    if (!fgets(path, sizeof(path), stdin)) {
        fprintf(stderr, 
                    "Failed to register input!\n");
        
        return -1;
    }

    path[strcspn(path, "\n")] = '\0'; // strip newline character

    // open file in reading mode
    FILE *fp = fopen(path, "r");
    
    if (!fp) {
        fprintf(stderr, 
                    "Failed to open file!\n");
        
        return -1;
    }

    // get the file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // buffer to hold data
    char *buffer = (char*)malloc(file_size * sizeof(char) + 1);
 
    if (!buffer) {
        fclose(fp);
        fprintf(stderr,     
                    "Memory allocation failed!\n");
        return -1;
    }

    // load data to buffer
    if (fread(buffer, sizeof(char), file_size, fp) != file_size) {
        fclose(fp);
        free(buffer);
        fprintf(stderr, 
                    "Failed to load file data!\n");
    
        return -1;
    }

    buffer[file_size] = '\0'; // nul terminate buffer
    fclose(fp); // close file

    // compress data
    const char *codes = compress(buffer);

    if (!codes) {
        free(buffer);
        fprintf(stderr, 
                    "Failed to compress input!\n");
        
        
        return -1;
    }

    free(buffer);

    // print it or you may store it
    printf("%s\n", codes);

    free((void*)codes);
    
    return 0;
}
