#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "con_term.h"
#include "con_parse.h"
#include "con_env.h"
#include "con_alloc.h"

void con_term_print(con_term_t*);

void con_term_print_pair(con_term_t*);

void con_term_print(con_term_t* t) {
    switch (t->type) {
        case FLONUM:
            printf("%f", t->value.flonum);
            break;
        case FIXNUM:
            printf("%ld", t->value.fixnum);
            break;
        case SYMBOL:
            printf("%s", t->value.sym.str);
            break;
        case LIST:
            printf("(");
            con_term_print_pair(t);
            printf(")");
            break;
        case EMPTY_LIST:
            printf("()");
            break;
        case BUILTIN:
            printf("<builtin-function>");
            break;
        case LAMBDA:
            printf("<lambda: %p>", t);
            break;
        default:
            puts("Printing unhandled for term.");
    }
}

void con_term_print_pair(con_term_t* t) {
    con_term_print(CAR(t));
    switch (CDR(t)->type) {
        case EMPTY_LIST:
            break;
        case LIST:
            printf(" ");
            con_term_print_pair(CDR(t));
            break;
        default:
            printf(" . ");
            con_term_print(CDR(t));
    }
}

enum {
    KWD_QUOTE,
    KWD_DEFINE,
    KWD_SET,
    KWD_LAMBDA,
    NUM_KEYWORDS
} KEYWORDS;

static con_term_t* keywords[NUM_KEYWORDS];

void init_keywords() {
    keywords[KWD_QUOTE]  = con_alloc_sym("quote");
    keywords[KWD_DEFINE] = con_alloc_sym("define");
    keywords[KWD_LAMBDA] = con_alloc_sym("lambda");
}

con_term_t* eval(con_env* env, con_term_t* list);

con_term_t* eval_args(con_env* env, con_term_t* list) {
    // Create the evaluated args by reversing and evaluating
    con_term_t *args, **a = &args;
    size_t length = list->value.list.length;

    for(con_term_t *t = list; t->type != EMPTY_LIST; t = CDR(t)) {
        *a = con_alloc(LIST);
        CAR(*a) = eval(env, CAR(t));
        (*a)->value.list.length = length--;
        a = &CDR(*a);
    }
    *a = con_alloc(EMPTY_LIST);
    args->value.list.length = length;
    return args;
}

void eval_define(con_env* env, con_term_t* t) {
    if (t->value.list.length != 2) {
        puts("ERROR: Invalid define form.");
    } else if (CAR(t)->type != SYMBOL) {
        puts("ERROR: Invalid define form, expected symbol.");
    }
    con_term_t* val = eval(env, CADR(t));
    if (val) {
        con_env_bind(env, CAR(t), val);
    }
}

con_term_t* eval_lambda(con_env* env, con_term_t* t) {
    if (t->value.list.length != 2) {
        puts("ERROR: Invalid lambda form.");
        return NULL;
    }
    con_term_t* vars = CAR(t);
    con_term_t* body = CADR(t);
    if (vars->type != LIST && vars->type != EMPTY_LIST) {
        puts("ERROR: Invalid lambda form, variables must be a list.");
        return NULL;
    }
    con_env* inner = con_env_init(env);

    con_term_t* lambda = con_alloc(LAMBDA);
    lambda->value.lambda.vars = vars;
    lambda->value.lambda.env  = inner;
    lambda->value.lambda.body = body;
    return lambda;
}

con_term_t* eval_lambda_call(con_term_t* lambda, con_term_t* args) {
    con_env* env     = lambda->value.lambda.env;
    con_term_t* vars = lambda->value.lambda.vars;
    int arity        = vars->value.list.length;
    int length       = args->value.list.length;
    if (length != arity) {
        printf("ERROR: Expected %d arguments, got %d.\n", arity, length);
        return NULL;
    }
    // Create the bindings in the lambda
    con_term_t *v, *b;
    while (vars->type != EMPTY_LIST) {
        v = CAR(vars);
        b = CAR(args);
        con_env_bind(env, v, b);
        vars = CDR(vars);
        args = CDR(args);
    }
    return eval(env, lambda->value.lambda.body);
}

con_term_t* eval(con_env* env, con_term_t* t) {
    int type = t->type;
    if (type == EMPTY_LIST || type == FIXNUM || type == FLONUM || type == LAMBDA) {
        // These types are self-evaluating
        return t;
    } else if (type == SYMBOL) {
        // resolve a lookup
        con_term_t* value;
        if ((value = con_env_lookup(env, t))) {
            return value;
        } else {
            printf("ERROR: Unbound variable '");
            con_term_print(t);
            printf("'.\n");
        }
    // Everything is a list from this point on
    } else if (CAR(t) == keywords[KWD_QUOTE]) {
        return CAR(CDR(t));
    } else if (CAR(t) == keywords[KWD_DEFINE]) {
        eval_define(env, CDR(t));
    } else if (CAR(t) == keywords[KWD_LAMBDA]) {
        return eval_lambda(env, CDR(t));
    } else {
        con_term_t *call = eval_args(env, t), *func, *args;

        func = CAR(call);
        args = CDR(call);

        if (func && func->type == BUILTIN) {
            return func->value.builtin(args);
        } else if (func && func->type == LAMBDA) {
            return eval_lambda_call(func, args);
        } else {
            puts("ERROR: First element of list must be a function");
            puts("ERROR: Could not evaluate the list.");
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    // Print version and exit information
    puts("con version 0.0.1");
    puts("Press Ctrl + C to Exit.\n");

    con_term_t* term = NULL;
    con_parser_t* parser = con_parser_init();

    con_alloc_init();
    con_env* global_env = con_env_init(NULL);
    con_env_add_builtins(global_env);

    init_keywords();

    char* input = NULL;
    while (1) {
        input = readline("con> ");
        add_history(input);

        if ((term = con_parser_parse(parser, "<stdin>", input))) {
            if ((term = eval(global_env, term))) {
                con_term_print(term);
                puts("");
            }
        }
        free(input);
    }

    free(input);
    con_alloc_deinit();
    con_env_destroy(global_env);

    return 0;
}

