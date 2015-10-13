#include <stdio.h>
#include <glib-2.0/glib.h>

#include "con_term.h"
#include "con_alloc.h"

void con_term_print_pair(con_term_t* t) {
    con_term_print(CAR(t));
    switch (CDR(t)->type) {
        case EMPTY_LIST:
            break;
        case LIST:
            printf(" ");
            con_term_print_pair(CDR(t));
            break;
        default:
            printf(" . ");
            con_term_print(CDR(t));
    }
}

void con_term_print(con_term_t* t) {
    // FIXME: There is a bug here that occured
    // when trying to print a root in eval_args
    // Also when sweeping an arena, although I think
    // that is because the type was not reset somehow
    // and it was still trying to print it.
    switch (t->type) {
        case FLONUM:
            printf("%f", t->value.flonum);
            break;
        case FIXNUM:
            printf("%ld", t->value.fixnum);
            break;
        case SYMBOL:
            printf("%s", t->value.sym.str);
            break;
        case LIST:
            printf("(");
            con_term_print_pair(t);
            printf(")");
            break;
        case EMPTY_LIST:
            printf("()");
            break;
        case BUILTIN:
            printf("<builtin-function>");
            break;
        case LAMBDA:
            printf("<lambda: %p>", t);
            break;
        case CON_TRUE:
            printf("true");
            break;
        case CON_FALSE:
            printf("false");
            break;
        case ENVIRONMENT:
            printf("<environment>");
            break;
        default:
            printf("???");
    }
}

void con_term_print_message(char* msg, con_term_t* t) {
    if (!t) {
        return;
    }
    fputs(msg, stdout);
    con_term_print(t);
    puts("");
}

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
