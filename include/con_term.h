#ifndef CON_TERM_H
#define CON_TERM_H
#include <stdlib.h>

typedef enum CON_TYPE {
    FIXNUM,
    FLONUM,
    LIST,
    SYMBOL,
    EMPTY_LIST,
    BUILTIN,
    CON_TRUE,
    CON_FALSE,
    ENVIRONMENT,
    UNDEFINED,
    LAMBDA
} CON_TYPE;

typedef struct con_term_t* (*con_builtin)(struct con_term_t*);

struct _GHashTable;

typedef struct con_term_t {
    CON_TYPE type;
    int mark:1;
    union {
        long fixnum;
        double flonum;
        con_builtin builtin;
        struct {
            struct _GHashTable* table;
            struct con_term_t* parent;
        } env;
        struct {
            struct con_term_t* car;
            struct con_term_t* cdr;
            size_t length;
        } list;
        struct {
            char* str;
            size_t size;
        } sym;
        struct {
            struct con_term_t* parent_env;
            struct con_term_t* vars;
            struct con_term_t* body;
        } lambda;
    } value;
} con_term_t;

void                con_env_init(con_term_t*, con_term_t* parent);
void                con_env_deinit(con_term_t*);
struct con_term_t*  con_env_lookup(con_term_t*, struct con_term_t*);
int                 con_env_bind(con_term_t*, struct con_term_t*, struct con_term_t*);
void                con_env_add_builtins(con_term_t*);

void                con_term_print(con_term_t*);
void                con_term_print_message(char*, con_term_t*);

con_term_t*         cons(con_term_t*, con_term_t*);
void                trace(con_term_t*);

#define CAR(t) ((t)->value.list.car)
#define CDR(t) ((t)->value.list.cdr)
#define CADR(t) (CAR(CDR(t)))
#define CADDR(t) (CAR(CDR(CDR(t))))

#define CON_LIST_FOREACH(I, L) \
    for (con_term_t *__list_iter__ = (L), *I = CAR(L);\
    __list_iter__->type != EMPTY_LIST;\
    __list_iter__ = CDR(__list_iter__), I = CAR(__list_iter__))

#endif /* end of include guard:  */
