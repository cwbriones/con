#include "con_term.h"
#include "con_alloc.h"

inline con_term_t* cons(con_term_t* first, con_term_t* rest) {
    con_term_t* pair = con_alloc(LIST);
    CAR(pair) = first;
    CDR(pair) = rest;

    return pair;
}
