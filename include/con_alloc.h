#ifndef CON_ALLOC_H
#define CON_ALLOC_H

struct con_term_t;

void   con_alloc_init();
void   con_alloc_deinit();
struct con_term_t* con_alloc(int);
struct con_term_t* con_alloc_sym(char*);
void   con_destroy(struct con_term_t*);

#endif // CON_ALLOC_H
