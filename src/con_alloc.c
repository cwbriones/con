#include "con_alloc.h"
#include "con_term.h"

con_term_t* con_alloc(int type) {
    con_term_t* term = malloc(sizeof(*term));
    term->type = type;
    return term;
}

void con_destroy(con_term_t* t) {
    if (!t) {
        return;
    }
    switch (t->type) {
        case SYMBOL:
            // Free the stored string
            free(t->value.sym.str);
            break;
        case LIST:
            // Recursively destroy
            con_destroy(CAR(t));
            con_destroy(CDR(t));
            break;
        default:
            break;
    }
    free(t);
}
