#ifndef CON_ALLOC_H
#define CON_ALLOC_H

struct con_term_t;

struct con_term_t* con_alloc(int);
void con_destroy(struct con_term_t*);

#endif // CON_ALLOC_H
