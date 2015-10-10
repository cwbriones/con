#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "con_term.h"
#include "con_parse.h"

void con_term_print(con_term_t*);

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
    // Print version and exit information
    puts("con version 0.0.1");
    puts("Press Ctrl + C to Exit.\n");

    con_term_t* term = NULL;
    con_parser_t* parser = con_parser_init();

    while (1) {
        char* input = readline("con> ");
        add_history(input);

        if ((term = con_parser_parse(parser, "<stdin>", input))) {
            con_term_print(term);
            puts("");
            con_term_destroy(term);
        }

        free(input);
    }

    return 0;
}

