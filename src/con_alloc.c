#include <string.h>
#include <stdio.h>

#include <glib-2.0/glib.h>

#include "con_alloc.h"
#include "con_term.h"

static GHashTable* con_symbols;
static con_term_t *con_true = NULL, *con_false = NULL;

void con_alloc_init() {
    con_symbols = g_hash_table_new(g_str_hash, g_str_equal);
    con_true    = con_alloc(CON_TRUE);
    con_false   = con_alloc(CON_FALSE);
}

void con_alloc_deinit() {
    g_hash_table_destroy(con_symbols);
    con_destroy(con_true);
    con_destroy(con_false);
}

con_term_t* con_alloc(int type) {
    con_term_t* term = malloc(sizeof(*term));
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
        s = con_alloc(SYMBOL);
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

void con_destroy(con_term_t* t) {
    if (!t) {
        return;
    }
    switch (t->type) {
        case SYMBOL:
            // Free the stored string
            free(t->value.sym.str);
            break;
        case ENVIRONMENT:
            con_env_deinit(t);
            break;
        case LIST:
            // Recursively destroy
            con_destroy(CAR(t));
            con_destroy(CDR(t));
            break;
        default:
            break;
    }
    free(t);
}

con_term_t* con_alloc_true() {
    return con_true;
}

con_term_t* con_alloc_false() {
    return con_false;
}
