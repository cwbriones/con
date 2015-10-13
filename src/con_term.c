#include <glib-2.0/glib.h>

#include "con_term.h"
#include "con_alloc.h"

void con_env_init(con_term_t* t, con_term_t* parent) {
    GHashTable* table = g_hash_table_new(g_str_hash, g_str_equal);
    t->value.env.parent = parent;
    t->value.env.table = table;
}

void con_env_deinit(con_term_t* t) {
    g_hash_table_destroy(t->value.env.table);
}

int con_env_bind(con_term_t* t, con_term_t* sym, con_term_t* val) {
    return g_hash_table_insert(t->value.env.table, sym->value.sym.str, val);
}

con_term_t* con_env_lookup(con_term_t* t, con_term_t* sym) {
    con_term_t* val;
    do {
        val = g_hash_table_lookup(t->value.env.table, sym->value.sym.str);
        t = t->value.env.parent;
    } while(!val && t);
    return val;
}

inline con_term_t* cons(con_term_t* first, con_term_t* rest) {
    con_term_t* pair = con_alloc(LIST);
    CAR(pair) = first;
    CDR(pair) = rest;

    return pair;
}

void trace(con_term_t* root) {
    /* if (root->mark) { */
    /*     return; */
    /* } */
    /* switch (root->type) { */
    /*     case LIST: */
    /*         trace(root->value.list.car); */
    /*         trace(root->value.list.cdr); */
    /*         break; */
    /*     case LAMBDA: */
    /*         trace(root->value.lambda.env); */
    /*         trace(root->value.lambda.vars); */
    /*         trace(root->value.lambda.body); */
    /*     default: */
    /*         break; */
    /* } */
    /* root->mark = 1; */
}
