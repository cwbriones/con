#include <glib-2.0/glib.h>

#include <stdio.h>

#include "con_env.h"
#include "con_term.h"
#include "con_alloc.h"

struct con_env {
    GHashTable* table;
    con_env* parent;
};

con_env* con_env_init(con_env* parent) {
    GHashTable* table = g_hash_table_new(g_str_hash, g_str_equal);
    con_env* env = malloc(sizeof(*env));

    env->parent = parent;
    env->table  = table;

    return env;
}

con_term_t* builtin_cons(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.\n", length);
        return NULL;
    }
    return cons(CAR(args), CAR(CDR(args)));
}

con_term_t* builtin_first(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 1) {
        printf("ERROR: Incorrect number of arguments, expected 1, got %zu.\n", length);
        return NULL;
    }
    con_term_t* l = CAR(args);
    if (args->type != LIST) {
        printf("ERROR: Expected list");
        return NULL;
    }
    return CAR(l);
}

con_term_t* builtin_rest(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 1) {
        printf("ERROR: Incorrect number of arguments, expected 1, got %zu.\n", length);
        return NULL;
    }
    con_term_t* l = CAR(args);
    if (args->type != LIST) {
        printf("ERROR: Expected list");
        return NULL;
    }
    return CDR(l);
}

con_term_t* builtin_add(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.\n", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) + (rhs->value.fixnum);
    return res;
}

con_term_t* builtin_sub(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) - (rhs->value.fixnum);
    return res;
}

con_term_t* builtin_mul(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) * (rhs->value.fixnum);
    return res;
}

con_term_t* builtin_div(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    if (rhs->value.fixnum == 0) {
        printf("ERROR: Expected list");
        return NULL;
    }
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) + (rhs->value.fixnum);
    return res;
}

void con_env_add_builtin(con_env* env, char* s, con_builtin builtin) {
    con_term_t* f = con_alloc(BUILTIN);
    con_term_t* sym = con_alloc_sym(s);
    f->value.builtin = builtin;

    con_env_bind(env, sym, f);
}

void con_env_add_builtins(con_env* env) {
    // List Functions
    con_env_add_builtin(env, "cons", builtin_cons);
    con_env_add_builtin(env, "first", builtin_first);
    con_env_add_builtin(env, "rest", builtin_rest);

    // Mathetatical Functions
    con_env_add_builtin(env, "+", builtin_add);
    con_env_add_builtin(env, "-", builtin_sub);
    con_env_add_builtin(env, "*", builtin_mul);
    con_env_add_builtin(env, "/", builtin_div);
}

void con_env_destroy(con_env* env) {
    g_hash_table_destroy(env->table);
    free(env);
}

int con_env_bind(con_env* env, con_term_t* sym, con_term_t* val) {
    return g_hash_table_insert(env->table, sym->value.sym.str, val);
}

con_term_t* con_env_lookup(con_env* env, con_term_t* sym) {
    con_term_t* val;
    do {
        val = g_hash_table_lookup(env->table, sym->value.sym.str);
        env = env->parent;
    } while(!val && env);
    return val;
}

