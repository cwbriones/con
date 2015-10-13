#include <string.h>
#include <stdio.h>

#include <glib-2.0/glib.h>

#include "con_alloc.h"
#include "con_term.h"

#define POOL_SIZE 100
#define POOL_ARENA_SIZE 1000

// Symbol table and singletons
static GHashTable* con_symbols = NULL;
static con_term_t *con_true = NULL, *con_false = NULL;

typedef struct {
    con_term_t* contents;
    size_t* free;
    size_t capacity;
    size_t size;
} arena;

arena* arena_init(size_t capacity) {
    puts("Initializing an arena.");
    arena* a = calloc(1, sizeof(*a));
    a->contents = calloc(capacity, sizeof(con_term_t));
    a->free = calloc(capacity, sizeof(size_t));
    for(int i = 0; i < capacity; i++) {
        a->free[i] = i;
    }
    a->capacity = capacity;
    a->size = 0;
    return a;
}

void arena_destroy(arena* a) {
    free(a->contents);
    free(a->free);
    free(a);
}

con_term_t* arena_alloc(arena* a) {
    if (a->capacity == a->size) {
        printf("FATAL: Arena out of memory");
    }
    // Pop the item off the free pool
    size_t idx = a->free[a->size++];
    return a->contents + idx;
}

void arena_free(arena* a, con_term_t* t) {
    if (!t) {
        return;
    }
    if (t->type == SYMBOL) {
        // Free the stored string
        free(t->value.sym.str);
    }
    // Push the index onto the stack
    size_t idx = t - a->contents;
    a->free[--a->size] = idx;
    return;
}

int arena_is_full(arena* a) {
    return a->capacity == 0;
}

typedef struct {
    arena** arenas;
    size_t capacity;
    size_t size;
} arena_pool;

arena_pool* arena_pool_init(size_t capacity) {
    puts("Initializing arena pool");
    arena_pool* p = calloc(1, sizeof(*p));
    arena **as = calloc(capacity, sizeof(*as));
    p->arenas = as;
    p->capacity = capacity;
    p->size = 0;
    return p;
}

void arena_pool_destroy(arena_pool* p) {
    for (int i = 0; i < p->capacity; i++){
        arena_destroy(p->arenas[i]);
    }
    free(p->arenas);
    free(p);
}

con_term_t* arena_pool_alloc(arena_pool* p) {
    for (int i = 0; i < p->size; i++) {
        if (!arena_is_full(p->arenas[i])) {
            return arena_alloc(p->arenas[i]);
        }
    }
    if (p->size == p->capacity) {
        realloc(p->arenas, ++(p->capacity) * sizeof(*p->arenas));
    }
    arena* a = arena_init(POOL_ARENA_SIZE);
    p->arenas[p->size++] = a;
    return arena_alloc(a);
}

static arena_pool *obj_pool = NULL;

void con_alloc_init() {
    obj_pool    = arena_pool_init(POOL_SIZE);
    con_symbols = g_hash_table_new(g_str_hash, g_str_equal);
    con_true    = con_alloc(CON_TRUE);
    con_false   = con_alloc(CON_FALSE);
}

void con_alloc_deinit() {
    arena_pool_destroy(obj_pool);
    g_hash_table_destroy(con_symbols);
    con_destroy(con_true);
    con_destroy(con_false);
}

con_term_t* con_alloc(int type) {
    con_term_t* term = arena_pool_alloc(obj_pool);
    term->type = type;
    if (type == EMPTY_LIST) {
        CAR(term) = NULL;
        CDR(term) = NULL;
        term->value.list.length = 0;
    }
    return term;
}

con_term_t* con_alloc_sym(char* sym) {
    con_term_t* s;
    if (!(s = g_hash_table_lookup(con_symbols, sym))){
        size_t l = strlen(sym);
        // Create the symbol
        s = malloc(sizeof(*s));
        s->type = SYMBOL;
        s->value.sym.str = malloc((l + 1) * sizeof(char));
        strcpy(s->value.sym.str, sym);
        s->value.sym.size = l;
        g_hash_table_insert(con_symbols, sym, s);
    }
    return s;
}

con_term_t* con_alloc_pair(con_term_t* fst, con_term_t* snd) {
    con_term_t *term, *term_cdr;
    term = con_alloc(LIST);
    term->value.list.length = 2;
    term_cdr = con_alloc(LIST);
    term_cdr->value.list.length = 1;

    CAR(term_cdr) = snd;
    CDR(term_cdr) = con_alloc(EMPTY_LIST);
    CAR(term) = fst;
    CDR(term) = term_cdr;

    return term;
}

con_term_t* con_alloc_env(con_term_t* parent) {
    con_term_t* t = con_alloc(ENVIRONMENT);
    con_env_init(t, parent);
    return t;
}

con_term_t* con_alloc_true() {
    return con_true;
}

con_term_t* con_alloc_false() {
    return con_false;
}

void con_gc() {
    // Mark all symbols
    // Mark all constants
    // Start from the root and mark everything else.
}
