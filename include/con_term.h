#ifndef CON_TERM_H
#define CON_TERM_H
#include <stdlib.h>

typedef enum CON_TYPE {
    FIXNUM,
    FLONUM,
    LIST,
    SYMBOL,
    EMPTY_LIST,
    BUILTIN
} CON_TYPE;

typedef struct con_term_t* (*con_builtin)(struct con_term_t*);

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
    } value;
} con_term_t;

#define CAR(t) ((t)->value.list.car)
#define CDR(t) ((t)->value.list.cdr)

con_term_t* cons(con_term_t*, con_term_t*);

#endif /* end of include guard:  */
