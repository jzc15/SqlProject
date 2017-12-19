
%{

#include <iostream>
#include <cstdlib>
#include "stmts.hpp"
#include "frontend_parse.h"

using namespace std;

#define SAVE_TOKEN yylval.str = new std::string(yytext, yyleng);
#define SAVE_TOKEN2 yylval.str = new std::string(yytext+1, yyleng-2)
#define SAVE_INT { string x(yytext, yyleng); yylval.int_value = atoi(x.c_str()); }
#define TOKEN(t) yylval.token = t
#define PRINT(t) { \
    printf("[%s] %s\n", std::string(yytext, yyleng).c_str(), #t); \
}


#ifndef PRINT
#define PRINT(t) { /*nothing*/ }
#endif

%}

%option noyywrap
%option yylineno

NEWLINE                 (\r|\n|\r\n)
WHITESPACE			    ([ \t]+)
IDENTIFIER              ([A-Za-z][_0-9A-Za-z]*)
VALUE_INT               ([0-9]+)
VALUE_STRING           '[^']*'
SIMPLE_OPERATOR         (";"|"`"|"'"|"("|")"|","|"*"|".")

%%

"DATABASE"              { PRINT(DATABASE); return TOKEN(DATABASE);      }
"DATABASES"             { PRINT(DATABASES); return TOKEN(DATABASES);    }
"TABLE"                 { PRINT(TABLE); return TOKEN(TABLE);            }
"TABLES"                { PRINT(TABLES); return TOKEN(TABLES);          }
"SHOW"                  { PRINT(SHOW); return TOKEN(SHOW);              }
"CREATE"                { PRINT(CREATE); return TOKEN(CREATE);          }
"DROP"                  { PRINT(DROP); return TOKEN(DROP);              }
"USE"                   { PRINT(USE); return TOKEN(USE);                }
"PRIMARY"               { PRINT(PRIMARY); return TOKEN(PRIMARY);        }
"KEY"                   { PRINT(KEY); return TOKEN(KEY);                }
"NOT"                   { PRINT(NOT); return TOKEN(NOT);                }
"NULL"                  { PRINT(TNULL); return TOKEN(TNULL);            }
"INSERT"                { PRINT(INSERT); return TOKEN(INSERT);          }
"INTO"                  { PRINT(INTO); return TOKEN(INTO);              }
"VALUES"                { PRINT(VALUES); return TOKEN(VALUES);          }
"DELETE"                { PRINT(DELETE); return TOKEN(DELETE);          }
"FROM"                  { PRINT(FROM); return TOKEN(FROM);              }
"WHERE"                 { PRINT(WHERE); return TOKEN(WHERE);            }
"UPDATE"                { PRINT(UPDATE); return TOKEN(UPDATE);          }
"SET"                   { PRINT(SET); return TOKEN(SET);                }
"SELECT"                { PRINT(SELECT); return TOKEN(SELECT);          }
"IS"                    { PRINT(IS); return TOKEN(IS);                  }
"INT"                   { PRINT(PINT); return TOKEN(PINT);              }
"VARCHAR"               { PRINT(VARCHAR); return TOKEN(VARCHAR);        }
"DESC"                  { PRINT(DESC); return TOKEN(DESC);              }
"INDEX"                 { PRINT(INDEX); return TOKEN(INDEX);            }
"AND"                   { PRINT(AND); return TOKEN(AND);                }
"DATE"                  { PRINT(DATE); return TOKEN(DATE);              }
"FLOAT"                 { PRINT(FLOAT); return TOKEN(FLOAT);            }
"FOREIGN"               { PRINT(FOREIGN); return TOKEN(FOREIGN);        }
"REFERENCES"            { PRINT(REFERENCES); return TOKEN(REFERENCES);  }
"<>"                    { PRINT(NEQ); return TOKEN(NEQ);                }
"<="                    { PRINT(LE); return TOKEN(LE);                  }
">="                    { PRINT(GE); return TOKEN(GE);                  }
"="                     { PRINT(EQ); return TOKEN(EQ);                  }
"<"                     { PRINT(LT); return TOKEN(LT);                  }
">"                     { PRINT(GT); return TOKEN(GT);                  }

{NEWLINE}               { /*skip*/                                                      }
{WHITESPACE}            { /*skip*/                                                      }
{IDENTIFIER}            { SAVE_TOKEN; PRINT(IDENTIFIER); return IDENTIFIER;             }
{VALUE_INT}             { SAVE_INT; PRINT(VALUE_INT); return VALUE_INT;                 }
{VALUE_STRING}          { SAVE_TOKEN2; PRINT(VALUE_STRING); return VALUE_STRING;        }
{SIMPLE_OPERATOR}       { return yytext[0];                                             }

.                       { printf("Unknown token [%c] at line %d!\n", yytext[0], yylineno); yyterminate(); exit(1); }

%%