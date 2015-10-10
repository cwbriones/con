#ifndef CON_ENV_H
#define CON_ENV_H

struct con_term_t;

typedef struct con_env con_env;

con_env*            con_env_init(con_env*);
void                con_env_destroy(con_env*);
struct con_term_t*  con_env_lookup(con_env*, struct con_term_t*);
int                 con_env_bind(con_env*, struct con_term_t*, struct con_term_t*);
void                con_env_add_builtins(con_env* env);

#endif // CON_ENV_H
