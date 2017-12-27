#include "helper.h"

#include <cstdio>
#include <iostream>
#include <cassert>

using namespace std;

extern FILE* yyin;

Program* pro = NULL;
bool scan_debug = false;

void yyerror(const char* s)
{
    printf("[yyerror:%d] %s\n", yylineno, s);
}

Program* parseFile(const char* filename)
{
    yyin = fopen(filename, "r");
    assert(yyin);
    pro = new Program();

    yyrestart(yyin);
    yyparse();

    fclose(yyin);

    return pro;
}

Program* parseFile(FILE *fd)
{
    yyin = fd;
    assert(yyin);
    pro = new Program();

    yyrestart(yyin);
    yyparse();

    return pro;
}

void scanFile(const char* filename)
{
    yyin = fopen(filename, "r");
    assert(yyin);
    while(yylex() > 0);
    fclose(yyin);
}
void scanFile(FILE *fd)
{
    yyin = fd;
    assert(yyin);
    while(yylex() >= 0);
}