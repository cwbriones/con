#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"

typedef enum CON_TYPE {
    FIXNUM,
    FLONUM,
    LIST,
    SYMBOL,
    EMPTY_LIST
} CON_TYPE;

typedef struct con_term_t {
    CON_TYPE type;
    union {
        long fixnum;
        double flonum;
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

con_term_t* make_empty_list() {
    con_term_t* t = malloc(sizeof(con_term_t));
    t->type = EMPTY_LIST;
    CAR(t) = NULL;
    CDR(t) = NULL;
    return t;
}

con_term_t* mpc_ast_to_term(mpc_ast_t* t);
void con_term_print(con_term_t*);

con_term_t* mpc_ast_to_list(mpc_ast_t* t) {
    con_term_t* list;
    int n = t->children_num;
    int i = n - 2;

    if (n > 3 && strcmp(t->children[n - 3]->contents, ".") == 0) {
        // Improper list
        list = mpc_ast_to_term(t->children[n - 2]);
        i = n - 4;
    } else {
        list = make_empty_list();
    }

    while (i > 0) {
        con_term_t* next = malloc(sizeof(*next));
        next->type = LIST;
        CAR(next) = mpc_ast_to_term(t->children[i]);
        CDR(next) = list;
        list = next;
        i--;
    }
    return list;
}

con_term_t* mpc_ast_to_term(mpc_ast_t* t) {
    con_term_t *term = NULL;
    if (t->children_num && strstr(t->children[0]->tag, "regex")) {
        t = t->children[1];
    }
    if (strstr(t->tag, "fixnum")) {
        term = malloc(sizeof(*term));
        term->type = FIXNUM;
        term->value.fixnum = atoi(t->contents);
    } else if (strstr(t->tag, "flonum")) {
        term = malloc(sizeof(*term));
        term->type = FLONUM;
        term->value.flonum = atof(t->contents);
    } else if (strstr(t->tag, "symbol")) {
        char* s = t->contents;
        size_t l = strlen(s);
        term = malloc(sizeof(*term));
        term->type = SYMBOL;

        // Create the symbol
        term->value.sym.str = malloc((l + 1) * sizeof(char));
        strcpy(term->value.sym.str, s);
        term->value.sym.size = l;
    } else if (strstr(t->tag, "list")) {
        term = mpc_ast_to_list(t);
    }
    return term;
}

void con_term_destroy(con_term_t* t) {
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
            con_term_destroy(CAR(t));
            con_term_destroy(CDR(t));
            break;
        default:
            break;
    }
    free(t);
}

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

int main(int argc, char** argv) {

    // Initialize the parsers
    mpc_parser_t* fixnum     = mpc_new("fixnum");
    mpc_parser_t* flonum     = mpc_new("flonum");
    mpc_parser_t* symbol     = mpc_new("symbol");
    mpc_parser_t* reserved   = mpc_new("reserved");
    mpc_parser_t* operator   = mpc_new("operator");
    mpc_parser_t* term       = mpc_new("term");
    mpc_parser_t* list       = mpc_new("list");
    mpc_parser_t* con        = mpc_new("con");

    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                        \
            fixnum    : /-?[0-9]+/ ;                             \
            flonum    : /-?[0-9]+\\.[0-9]*/ ;                    \
            reserved  : \"lambda\" | \"let\" | \"define\" |      \
                        \"set\";                                 \
            operator  : '+' | '-' | '*' | '/' ;                  \
            symbol    : <operator> | /[_a-zA-Z][_a-zA-Z0-9]*/ ;  \
            term      : <flonum> | <fixnum> | <symbol> | <list>; \
            list      : '(' <term>* ('.' <term>)? ')';           \
            con       : /^/ <term>  /$/   ;                      \
        ",
        fixnum, flonum, reserved, operator, symbol, term, list, con);

    // Print version and exit information
    puts("con version 0.0.1");
    puts("Press Ctrl + C to Exit.\n");

    while (1) {
        char* input = readline("con> ");
        add_history(input);

        mpc_result_t res;
        if (mpc_parse("<stdin>", input, con, &res)) {
            con_term_t* term = mpc_ast_to_term(res.output);
            if (term) {
                con_term_print(term);
                puts("");
            } else {
                puts("Parse unhandled for term.");
                mpc_ast_print(res.output);
            }
            con_term_destroy(term);
            mpc_ast_delete(res.output);
        } else {
            mpc_err_print(res.error);
            mpc_err_delete(res.error);
        }

        free(input);
    }

    mpc_cleanup(8,
        flonum,
        fixnum,
        symbol,
        reserved,
        operator,
        term,
        list,
        con);
    return 0;
}

