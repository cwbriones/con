#include "con_term.h"
#include "con_parse.h"
#include "con_alloc.h"

#include "mpc.h"

#define NUM_PARSERS 8

struct con_parser_t {
    mpc_parser_t* con;
    mpc_parser_t* rest[NUM_PARSERS];
};

con_term_t* mpc_ast_to_list(mpc_ast_t* t);
con_term_t* mpc_ast_to_term(mpc_ast_t* t);

con_parser_t* con_parser_init() {
    // Initialize the parsers
    mpc_parser_t* fixnum     = mpc_new("fixnum");
    mpc_parser_t* flonum     = mpc_new("flonum");
    mpc_parser_t* reserved   = mpc_new("reserved");
    mpc_parser_t* operator   = mpc_new("operator");
    mpc_parser_t* symbol     = mpc_new("symbol");
    mpc_parser_t* term       = mpc_new("term");
    mpc_parser_t* list       = mpc_new("list");
    mpc_parser_t* quote      = mpc_new("quote");
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
            quote     : \'\'\' <term> ;                          \
            con       : /^/ <term>  /$/ | <quote> ;              \
        ",
        fixnum, flonum, reserved, operator, symbol, term, list, quote, con);

    con_parser_t* parser = malloc(sizeof(*parser));
    parser->con = con;

    parser->rest[0] = fixnum;
    parser->rest[1] = flonum;
    parser->rest[2] = reserved;
    parser->rest[3] = operator;
    parser->rest[4] = symbol;
    parser->rest[5] = term;
    parser->rest[6] = list;
    parser->rest[7] = quote;

    return parser;
}

con_term_t* con_parser_parse(con_parser_t* parser, char* tag, char* input) {
    mpc_result_t res;
    con_term_t* term = NULL;

    if (mpc_parse(tag, input, parser->con, &res)) {
        term = mpc_ast_to_term(res.output);
        if (!term) {
            puts("Parse unhandled for term.");
            mpc_ast_print(res.output);
        }
        mpc_ast_delete(res.output);
    } else {
        mpc_err_print(res.error);
        mpc_err_delete(res.error);
    }
    return term;
}

void con_parser_destroy(con_parser_t* parser) {
    mpc_cleanup(1, parser->con);
    for (int i = 0; i < NUM_PARSERS; i++) {
        mpc_cleanup(1, parser->rest[i]);
    }
}

con_term_t* mpc_ast_to_term(mpc_ast_t* t) {
    con_term_t *term = NULL;
    if (t->children_num && strstr(t->children[0]->tag, "regex")) {
        t = t->children[1];
    }
    if (t->children_num && strstr(t->children[0]->tag, "quote")) {
        // Quote is of the form ' <term>, so the first child
        // is the ' and the second the actual term.
        t = t->children[0]->children[1];
        term = con_alloc_pair(con_alloc_sym("quote"), mpc_ast_to_term(t));
    } else if (strstr(t->tag, "fixnum")) {
        term = con_alloc(FIXNUM);
        term->value.fixnum = atoi(t->contents);
    } else if (strstr(t->tag, "flonum")) {
        term = con_alloc(FLONUM);
        term->value.flonum = atof(t->contents);
    } else if (strstr(t->tag, "symbol")) {
        term = con_alloc_sym(t->contents);
    } else if (strstr(t->tag, "list")) {
        term = mpc_ast_to_list(t);
    }
    return term;
}

con_term_t* mpc_ast_to_list(mpc_ast_t* t) {
    con_term_t* list;
    int n = t->children_num;
    int i = n - 2;

    if (i > 1 && strcmp(t->children[i-1]->contents, ".") == 0) {
        // Improper list
        list = mpc_ast_to_term(t->children[i]);
        i = n - 4;
    } else {
        list = con_alloc(EMPTY_LIST);
    }

    size_t length = 0;
    while (i > 0) {
        con_term_t* next = con_alloc(LIST);
        CAR(next) = mpc_ast_to_term(t->children[i]);
        CDR(next) = list;
        next->value.list.length = (++length);
        list = next;
        i--;
    }
    return list;
}

