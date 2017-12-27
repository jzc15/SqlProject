#ifndef _PARSE_H
#define _PARSE_H

#include <cstdio>
#include "stmts.hpp"
#include "program.hpp"

extern Program* pro;
extern bool scan_debug;
extern int yylineno;

// need by yacc
void yyerror(const char* s);
int yylex(void); // defined in frontend_lex.cc(scan.l)
int yyparse(void); // defined in frontend_parse.cc(parse.y)
void yyrestart(FILE* in);

Program* parseFile(const char* filename);
Program* parseFile(FILE *fd = stdin);
void scanFile(const char* filename);
void scanFile(FILE *fd = stdin);

#endif // _PARSE_H