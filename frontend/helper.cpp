#include "helper.h"

#include <cstdio>
#include <iostream>
#include <assert.h>

using namespace std;

extern FILE* yyin;

void yyerror(const char* s)
{
    puts(s);
}

void parseFile(const char* filename)
{
    yyin = fopen(filename, "r");
    assert(yyin);
    yyparse();
}

void parseFile(FILE *fd)
{
    yyin = fd;
    yyparse();
}