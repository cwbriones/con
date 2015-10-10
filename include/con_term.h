#ifndef CON_TERM_H
#define CON_TERM_H
#include <stdlib.h>

typedef enum CON_TYPE {
    FIXNUM,
    FLONUM,
    LIST,
    SYMBOL,
    EMPTY_LIST,
    FUNCTION
} CON_TYPE;

typedef struct con_term_t* (*con_function)(struct con_term_t*);

typedef struct con_term_t {
    CON_TYPE type;
    union {
        long fixnum;
        double flonum;
        con_function function;
        struct {
            struct con_term_t* car;
            struct con_term_t* cdr;
        } list;
        struct {
            char* str;
            size_t size;
        } sym;
    } value;
} con_term_t;

#define CAR(t) (t->value.list.car)
#define CDR(t) (t->value.list.cdr)

#endif /* end of include guard:  */
