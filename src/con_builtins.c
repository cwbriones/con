#include <stdio.h>

#include "con_term.h"
#include "con_builtins.h"
#include "con_alloc.h"

con_term_t* builtin_cons(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.\n", length);
        return NULL;
    }
    return cons(CAR(args), CAR(CDR(args)));
}

con_term_t* builtin_first(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 1) {
        printf("ERROR: Incorrect number of arguments, expected 1, got %zu.\n", length);
        return NULL;
    }
    con_term_t* l = CAR(args);
    if (args->type != LIST) {
        printf("ERROR: Expected list");
        return NULL;
    }
    return CAR(l);
}

con_term_t* builtin_rest(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 1) {
        printf("ERROR: Incorrect number of arguments, expected 1, got %zu.\n", length);
        return NULL;
    }
    con_term_t* l = CAR(args);
    if (args->type != LIST) {
        printf("ERROR: Expected list");
        return NULL;
    }
    return CDR(l);
}

con_term_t* builtin_add(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.\n", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) + (rhs->value.fixnum);
    return res;
}

con_term_t* builtin_sub(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) - (rhs->value.fixnum);
    return res;
}

con_term_t* builtin_mul(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) * (rhs->value.fixnum);
    return res;
}

con_term_t* builtin_div(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args)), *res = NULL;
    if (rhs->value.fixnum == 0) {
        printf("ERROR: Expected list");
        return NULL;
    }
    res = con_alloc(FIXNUM);
    res->value.fixnum = (lhs->value.fixnum) + (rhs->value.fixnum);
    return res;
}

con_term_t* builtin_equals(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args));
    if (lhs->type != rhs->type) {
        return con_alloc_false();
    }
    int result;
    switch (lhs->type) {
        case FIXNUM:
            result = lhs->value.fixnum == rhs->value.fixnum;
            break;
        case FLONUM:
            result = lhs->value.flonum == rhs->value.flonum;
            break;
        case EMPTY_LIST:
            result = 1;
            break;
        case SYMBOL:
        case BUILTIN:
        case LAMBDA:
            result = lhs == rhs;
            break;
        default:
            // TODO: Proper list comparison
            result = 0;
    }
    if (result) {
        return con_alloc_true();
    }
    return con_alloc_false();
}

con_term_t* builtin_is(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args));
    return lhs == rhs ? con_alloc_true() : con_alloc_false();
}

con_term_t* builtin_less_than(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CAR(CDR(args));
    if (lhs->type == rhs->type) {
        if (lhs->type == FIXNUM) {
            return lhs->value.fixnum < rhs->value.fixnum ? con_alloc_false() : con_alloc_true();
        } else if (lhs->type == FLONUM) {
            return lhs->value.flonum < rhs->value.flonum ? con_alloc_false() : con_alloc_true();
        }
    }
    printf("ERROR: Cannot compare arguments.");
    return NULL;
}

con_term_t* builtin_greater_than(con_term_t* args) {
    size_t length = args->value.list.length;
    if (length != 2) {
        printf("ERROR: Incorrect number of arguments, expected 2, got %zu.", length);
        return NULL;
    }
    con_term_t *lhs = CAR(args), *rhs = CADR(args);
    if (lhs->type == rhs->type) {
        if (lhs->type == FIXNUM) {
            return lhs->value.fixnum > rhs->value.fixnum ? con_alloc_sym("false") : con_alloc_sym("true");
        } else if (lhs->type == FLONUM) {
            return lhs->value.flonum > rhs->value.flonum ? con_alloc_sym("false") : con_alloc_sym("true");
        }
    }
    printf("ERROR: Cannot compare arguments.");
    return NULL;
}

void con_env_add_builtin(con_term_t* env, char* s, con_builtin builtin) {
    con_term_t* f = con_alloc(BUILTIN);
    con_term_t* sym = con_alloc_sym(s);
    f->value.builtin = builtin;

    con_env_bind(env, sym, f);
}

void con_env_add_builtins(con_term_t* env) {
    // List Functions
    con_env_add_builtin(env, "cons", builtin_cons);
    con_env_add_builtin(env, "first", builtin_first);
    con_env_add_builtin(env, "rest", builtin_rest);

    // Mathematical Functions
    con_env_add_builtin(env, "+", builtin_add);
    con_env_add_builtin(env, "-", builtin_sub);
    con_env_add_builtin(env, "*", builtin_mul);
    con_env_add_builtin(env, "/", builtin_div);

    // Predicates
    con_env_add_builtin(env, "is?", builtin_is);
    con_env_add_builtin(env, "eq?", builtin_equals);
    con_env_add_builtin(env, ">", builtin_less_than);
    con_env_add_builtin(env, "<", builtin_greater_than);
}

