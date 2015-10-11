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
    LAMBDA
} CON_TYPE;

typedef struct con_term_t* (*con_builtin)(struct con_term_t*);

struct con_env;

typedef struct con_term_t {
    CON_TYPE type;
    union {
        long fixnum;
        double flonum;
        con_builtin builtin;
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
            struct con_env* env;
            struct con_term_t* vars;
            struct con_term_t* body;
        } lambda;
    } value;
} con_term_t;

#define CAR(t) ((t)->value.list.car)
#define CDR(t) ((t)->value.list.cdr)
#define CADR(t) (CAR(CDR(t)))

con_term_t* cons(con_term_t*, con_term_t*);

#endif /* end of include guard:  */
