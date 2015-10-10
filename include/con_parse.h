#ifndef PARSE_H
#define PARSE_H

typedef struct con_parser_t con_parser_t;

struct con_parser_t* con_parser_init();
struct con_term_t* con_parser_parse(struct con_parser_t*, char*, char*);
void con_parser_destroy(struct con_parser_t*);

#endif /* end of include guard: PARSE_H */
