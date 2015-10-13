#ifndef CON_ALLOC_H
#define CON_ALLOC_H

struct con_term_t;

void   con_alloc_init();
void   con_alloc_deinit();

struct con_term_t* con_alloc(int);
struct con_term_t* con_alloc_sym(char*);
struct con_term_t* con_alloc_true();
struct con_term_t* con_alloc_false();
struct con_term_t* con_alloc_pair(struct con_term_t*, struct con_term_t*);
struct con_term_t* con_alloc_env(struct con_term_t*);

void   con_root(struct con_term_t**);
void   con_unroot(struct con_term_t**);
void   con_gc();
#endif // CON_ALLOC_H
