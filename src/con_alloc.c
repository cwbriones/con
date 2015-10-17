#include <string.h>
#include <stdio.h>

#include <glib-2.0/glib.h>

#include "con_alloc.h"
#include "con_term.h"

#define POOL_SIZE 100
#define POOL_ARENA_SIZE 1000

#ifdef GC_DEBUG
#define INIT_GC_ALLOC_TRIGGER 1
#define GC_ALLOC_TRIGGER 1
#else
#define INIT_GC_ALLOC_TRIGGER 500
#define GC_ALLOC_TRIGGER 100
#endif

// Needed for garbage collection
static size_t allocations_since_gc = 0;
static int initial_gc = 0;

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
    arena* a = calloc(1, sizeof(*a));
    a->contents = calloc(capacity, sizeof(con_term_t));
    a->free = calloc(capacity, sizeof(size_t));
    for(int i = 0; i < capacity; i++) {
        a->free[i] = i;
        a->contents[i].mark = 0;
        a->contents[i].type = UNDEFINED;
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

void arena_sweep(arena* a) {
    if (a->size == 0) {
        return;
    }
    con_term_t* t;
#ifdef GC_DEBUG
    puts("Sweeping an arena");
#endif
    for (int i = 0; i < a->size; i++) {
        t = a->contents + i;
        if (!t->mark && t->type != UNDEFINED) {
            // push this index onto the stack
#ifdef GC_DEBUG
            printf("Sweep: %p\n", (void*) t);
#endif
            if (t->type == ENVIRONMENT) {
                con_env_deinit(t);
            }
            t->type = UNDEFINED;
            a->free[--a->size] = i;
        }
    }
    // TODO: See if we can make this more specific
    // Since we shouldn't have to reset **everything**
    for (int i = 0; i < a->capacity; i++) {
        a->contents[i].mark = 0;
    }
#ifdef GC_DEBUG
    puts("Finished with an arena");
#endif
}

int arena_is_full(arena* a);

inline int arena_is_full(arena* a) {
    return a->capacity == a->size;
}

typedef struct {
    arena** arenas;
    size_t capacity;
    size_t size;
} arena_pool;

arena_pool* arena_pool_init(size_t capacity) {
    arena_pool* p = calloc(1, sizeof(*p));
    arena **as = calloc(capacity, sizeof(*as));
    p->arenas = as;
    p->capacity = capacity;
    p->size = 0;
    return p;
}

void arena_pool_destroy(arena_pool* p) {
    for (int i = 0; i < p->size; i++){
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

void arena_pool_sweep(arena_pool* p) {
    size_t total = 0;
    for (int i = 0; i < p->size; i++) {
        arena_sweep(p->arenas[i]);
        total += p->arenas[i]->size;
    }
}

static arena_pool *obj_pool = NULL;

void symbol_destroy(void* t) {
    con_term_t *sym = t;
    free(sym->value.sym.str);
    free(sym);
}

void con_alloc_init() {
    obj_pool    = arena_pool_init(POOL_SIZE);
    con_symbols = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, symbol_destroy);
    con_true    = malloc(sizeof(*con_true));
    con_true->type = CON_TRUE;
    con_false   = malloc(sizeof(*con_false));
    con_false->type = CON_FALSE;
}

void destroy_roots();

void con_alloc_deinit() {
    // Free singleton objects
    free(con_true);
    free(con_false);
    destroy_roots();
    arena_pool_destroy(obj_pool);
    g_hash_table_destroy(con_symbols);
}

void con_gc();

con_term_t* con_alloc(int type) {
    con_term_t* term = arena_pool_alloc(obj_pool);
    term->type = type;
    if (type == EMPTY_LIST) {
        CAR(term) = NULL;
        CDR(term) = NULL;
        term->value.list.length = 0;
    }
    allocations_since_gc += 1;
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

typedef struct root {
    con_term_t** t;
    struct root* next;
} root;

static root* roots = NULL;

size_t count_roots() {
    size_t count = 0;
    for (root *r = roots; r != NULL; r = r->next) {
        count++;
    }
    return count;
}

void con_gc() {
    if ((!initial_gc && allocations_since_gc < INIT_GC_ALLOC_TRIGGER) ||
        (allocations_since_gc < GC_ALLOC_TRIGGER)) {
        return;
    }
    allocations_since_gc = 0;
    initial_gc = 1;
    root* r = roots;
#ifdef GC_DEBUG
    puts("\nGC Running.");
    printf("There are %lu roots.\n", count_roots());
    puts("Marky mark");
#endif
    while (r != NULL) {
        if (*r->t) {
            trace(*r->t);
        }
        r = r->next;
    }
#ifdef GC_DEBUG
    puts("Sweepy sweep.");
#endif
    arena_pool_sweep(obj_pool);
#ifdef GC_DEBUG
    puts("GC run complete.");
#endif
}

void trace(con_term_t*);

void mark_environment_values(GHashTable* g) {
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, g);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        con_term_t* t = value;
        trace(t);
    }
}

void trace(con_term_t* t) {
    if (!t || t->mark) {
        return;
    }
#ifdef GC_DEBUG
    /* printf("Tracing: %p\n", (void*)t); */
#endif
    t->mark = 1;
    if (t->type == LIST) {
        trace(t->value.list.car);
        trace(t->value.list.cdr);
    } else if (t->type == LAMBDA) {
        trace(t->value.lambda.vars);
        trace(t->value.lambda.body);
        trace(t->value.lambda.parent_env);
    } else if (t->type == ENVIRONMENT) {
        mark_environment_values(t->value.env.table);
    }
}

void con_root(con_term_t **t) {
#ifdef GC_DEBUG
    printf("Rooting object:   %p\n", (void*)(t));
#endif
    root* r = malloc(sizeof(*r));
    r->t = t;
    r->next = roots;
    roots = r;
}

void con_unroot(con_term_t **t) {
#ifdef GC_DEBUG
    if (*t) {
        printf("Unrooting object: %p\n", (void*)(t));
    } else {
        puts("Unrooting a currently NULL object.");
    }
#endif
    root **r;
    for (r = &roots; *r != NULL; r = &(*r)->next) {
        if ((*r)->t == t) {
            root* next = (*r)->next;
            free(*r);
            *r = next;
            return;
        }
    }
    puts("UNREACHABLE!");
    *NULL;
}

void destroy_roots() {
    root *r = roots, *p = NULL;
    while (r) {
        p = r;
        r = r->next;
        free(p);
    }
}
