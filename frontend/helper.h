#ifndef _PARSE_H
#define _PARSE_H

#include <cstdio>
#include "nodes.hpp"
#include "program.hpp"

extern Program* pro;

// need by yacc
void yyerror(const char* s);
int yylex(void); // defined in frontend_lex.cc(scan.l)
int yyparse(void); // defined in frontend_parse.cc(parse.y)

Program* parseFile(const char* filename);
Program* parseFile(FILE *fd = stdin);

#endif // _PARSE_H