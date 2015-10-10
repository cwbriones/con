#include <string.h>

#include <glib-2.0/glib.h>

#include "con_alloc.h"
#include "con_term.h"

GHashTable* con_symbols;

void con_alloc_init() {
    con_symbols = g_hash_table_new(g_str_hash, g_str_equal);
}

void con_alloc_deinit() {
    g_hash_table_destroy(con_symbols);
}

con_term_t* con_alloc(int type) {
    con_term_t* term = malloc(sizeof(*term));
    term->type = type;
    return term;
}

con_term_t* con_alloc_sym(char* sym) {
    con_term_t* s;
    if (!(s = g_hash_table_lookup(con_symbols, sym))){
        size_t l = strlen(sym);
        // Create the symbol
        s->value.sym.str = malloc((l + 1) * sizeof(char));
        strcpy(s->value.sym.str, sym);
        s->value.sym.size = l;
        g_hash_table_insert(con_symbols, sym, s);
    }
    return s;
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
