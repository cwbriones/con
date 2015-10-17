#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "con_term.h"
#include "con_parse.h"
#include "con_alloc.h"
#include "con_builtins.h"

enum {
    KWD_QUOTE,
    KWD_DEFINE,
    KWD_SET,
    KWD_LAMBDA,
    KWD_IF,
    NUM_KEYWORDS
} KEYWORDS;

static con_term_t* keywords[NUM_KEYWORDS];

void init_keywords() {
    keywords[KWD_QUOTE]  = con_alloc_sym("quote");
    keywords[KWD_DEFINE] = con_alloc_sym("define");
    keywords[KWD_LAMBDA] = con_alloc_sym("lambda");
    keywords[KWD_IF]     = con_alloc_sym("if");
}

con_term_t current_thunk;

con_term_t* eval(con_term_t* env, con_term_t* list);
con_term_t* thunk(con_term_t* env, con_term_t* code);

con_term_t* eval_args(con_term_t* env, con_term_t* list) {
    con_root(&list);
    for(con_term_t* t = list; t->type != EMPTY_LIST; t = CDR(t)) {
        CAR(t) = eval(env, CAR(t));
    }
    con_unroot(&list);
    return list;
}

void eval_define(con_term_t* env, con_term_t* t) {
    con_term_t* val = NULL;
    if (t->value.list.length == 2 && CAR(t)->type == SYMBOL) {
        val = eval(env, CADR(t));
    } else if (t->value.list.length >= 2 && CAR(t)->type == LIST) {
        con_term_t* vars = CDR(CAR(t));
        con_term_t* body = CADR(t);
        t = CAR(t);
        val = con_alloc(LAMBDA);
        val->value.lambda.vars = vars;
        val->value.lambda.body = body;
        val->value.lambda.parent_env = env;
    }
    if (val) {
        con_env_bind(env, CAR(t), val);
    } else {
        puts("ERROR: Invalid define form.");
    }
}

con_term_t* eval_lambda(con_term_t* env, con_term_t* t) {
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

    con_term_t* lambda = con_alloc(LAMBDA);
    lambda->value.lambda.vars = vars;
    lambda->value.lambda.body = body;
    lambda->value.lambda.parent_env = env;
    return lambda;
}

con_term_t* eval_lambda_call(con_term_t* lambda, con_term_t* args) {
    con_term_t* inner = con_alloc_env(lambda->value.lambda.parent_env);
    con_term_t* vars  = lambda->value.lambda.vars;
    int arity         = vars->value.list.length;
    int length        = args->value.list.length;
    if (length != arity) {
        printf("ERROR: Expected %d arguments, got %d.\n", arity, length);
        return NULL;
    }
    // Create the bindings in the lambda
    con_term_t *v, *b;
    while (vars->type != EMPTY_LIST) {
        v = CAR(vars);
        b = CAR(args);
        con_env_bind(inner, v, b);
        vars = CDR(vars);
        args = CDR(args);
    }
    return thunk(inner, lambda->value.lambda.body);
}

con_term_t* eval_list_trampoline(con_term_t* env, con_term_t* t) {
    con_term_t *first = CAR(t);
    t = CDR(t);
    if (first == keywords[KWD_QUOTE]) {
        return CAR(t);
    } else if (first == keywords[KWD_DEFINE]) {
        eval_define(env, t);
    } else if (first == keywords[KWD_LAMBDA]) {
        return eval_lambda(env, t);
    } else if (first == keywords[KWD_IF]) {
        if (t->value.list.length != 3) {
            puts("ERROR: Invalid 'if' form.");
            return NULL;
        }
        con_term_t *cond, *body, *res;
        cond = CAR(t);
        // Initialize body to false
        body = CADDR(t);
        res = eval(env, cond);
        if (res && res->type == CON_TRUE) {
            body = CADR(t);
        }
        return eval(env, body);
    } else {
        con_term_t *args = eval_args(env, t), *func;
        // Need to root args here since evaluating the func can be bad.
        con_root(&args);
        func = eval(env, first);
        con_unroot(&args);
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

con_term_t* eval_list(con_term_t* env, con_term_t* t) {
    con_term_t* result = eval_list_trampoline(env, t);
    while (result == &current_thunk) {
        env = CAR(&current_thunk);
        t   = CDR(&current_thunk);
        con_root(&env);
        result = eval_list_trampoline(env, t);
        con_unroot(&env);
    }
    return result;
}

con_term_t* thunk(con_term_t* env, con_term_t* code) {
    if (code->type != LIST) {
        return eval(env, code);
    }
    current_thunk.type = LIST;
    CAR(&current_thunk) = env;
    CDR(&current_thunk) = code;
    return &current_thunk;
}

con_term_t* eval(con_term_t* env, con_term_t* t) {
    con_gc();
    if (t->type == SYMBOL) {
        // resolve a lookup
        con_term_t* value;
        if ((value = con_env_lookup(env, t))) {
            return value;
        } else {
            printf("ERROR: Unbound variable '");
            con_term_print(t);
            printf("'.\n");
            return NULL;
        }
        return NULL;
    } else if (t->type != LIST) {
        return t;
    }
    return eval_list(env, t);
}

static int done = 0;

void handle_signals(int signo) {
    if (signo == SIGINT) {
        done = 1;
    }
}

int main(int argc, char** argv) {
    // Print version and exit information
    puts("con version 0.0.1");
    puts("Press Ctrl + C to Exit.\n");

    if (signal(SIGINT, handle_signals) == SIG_ERR) {
        puts("FATAL: failed to register interrupts with kernel.");
        exit(1);
    }

    con_term_t* term = NULL;
    con_parser_t* parser = con_parser_init();

    con_alloc_init();
    con_term_t* global_env = con_alloc_env(NULL);
    con_root(&global_env);
    con_root(&term);
    con_env_add_builtins(global_env);
    init_keywords();

    char* input = NULL;
    while (1) {
        input = readline("con> ");
        if (!done) {
            add_history(input);
            if ((term = con_parser_parse(parser, "<stdin>", input))) {
                if ((term = eval(global_env, term))) {
                    con_term_print(term);
                    puts("");
                }
            }
            free(input);
        } else {
            break;
        }
    }
    clear_history();
    rl_cleanup_after_signal();
    con_alloc_deinit();
    con_parser_destroy(parser);

    return 0;
}

