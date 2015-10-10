#include <glib-2.0/glib.h>

#include "con_env.h"
#include "con_term.h"

struct con_env {
    GHashTable* table;
    con_env* parent;
};

con_env* con_env_init(con_env* parent) {
    GHashTable* table = g_hash_table_new(g_str_hash, g_str_equal);
    con_env* env = malloc(sizeof(*env));

    env->parent = parent;
    env->table  = table;
    env->parent = NULL;

    return env;
}

void con_env_destroy(con_env* env) {
    g_hash_table_destroy(env->table);
    free(env);
}

int con_env_bind(con_env* env, char* sym, con_term_t* val) {
    return g_hash_table_insert(env->table, sym, val);
}

con_term_t* con_env_lookup(con_env* env, char* sym) {
    con_term_t* val;
    do {
        val = g_hash_table_lookup(env->table, sym);
        env = env->parent;
    } while(val && env);
    return val;
}

