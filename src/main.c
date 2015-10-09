#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"

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
            list      : '(' <term>* ')'   ;                      \
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
            mpc_ast_print(res.output);
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
