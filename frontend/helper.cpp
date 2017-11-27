#include "helper.h"

#include <cstdio>
#include <iostream>
#include <assert.h>

using namespace std;

extern FILE* yyin;

Program* pro = NULL;

void yyerror(const char* s)
{
    puts(s);
}

Program* parseFile(const char* filename)
{
    yyin = fopen(filename, "r");
    assert(yyin);
    yyparse();

    Program* t = pro;
    pro = NULL;
    return t;
}

Program* parseFile(FILE *fd)
{
    yyin = fd;
    yyparse();

    Program* t = pro;
    pro = NULL;
    return t;
}