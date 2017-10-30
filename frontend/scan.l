
%{

#include <iostream>
#include "frontend_parse.h"

using namespace std;

#define SAVE_TOKEN yylval.str = new std::string(yytext, yyleng)
#define SAVE_TOKEN2 yylval.str = new std::string(yytext+1, yyleng-2)
#define TOKEN(t) ( \
    printf("[%s] %s\n", std::string(yytext, yyleng).c_str(), #t), \
    yylval.token = t \
)

%}

%option noyywrap
%option yylineno

NEWLINE                 (\r|\n|\r\n)
WHITESPACE			    ([ \t]+)
IDENTIFIER              ([A-Za-z][_0-9A-Za-z]*)
IDENTIFIER2             `([A-Za-z][_0-9A-Za-z]*)`
VALUE_INT               ([0-9]+)
VALUE_STRING1           '[^']*'
VALUE_STRING2           \"[^\"]*\"
SIMPLE_OPERATOR         (";"|"`"|"'"|"("|")"|",")

%%

"DATABASE"              { return TOKEN(DATABASE);       }
"DATABASES"             { return TOKEN(DATABASES);      }
"TABLE"                 { return TOKEN(TABLE);          }
"TABLES"                { return TOKEN(TABLES);         }
"SHOW"                  { return TOKEN(SHOW);           }
"CREATE"                { return TOKEN(CREATE);         }
"DROP"                  { return TOKEN(DROP);           }
"USE"                   { return TOKEN(USE);            }
"PRIMARY"               { return TOKEN(PRIMARY);        }
"KEY"                   { return TOKEN(KEY);            }
"NOT"                   { return TOKEN(NOT);            }
"NULL"                  { return TOKEN(TNULL);          }
"INSERT"                { return TOKEN(INSERT);         }
"INTO"                  { return TOKEN(INTO);           }
"VALUES"                { return TOKEN(VALUES);         }
"DELETE"                { return TOKEN(DELETE);         }
"FROM"                  { return TOKEN(FROM);           }
"WHERE"                 { return TOKEN(WHERE);          }
"UPDATE"                { return TOKEN(UPDATE);         }
"SET"                   { return TOKEN(SET);            }
"SELECT"                { return TOKEN(SELECT);         }
"IS"                    { return TOKEN(IS);             }
"INT"                   { return TOKEN(INT);            }
"VARCHAR"               { return TOKEN(VARCHAR);        }
"DESC"                  { return TOKEN(DESC);           }
"INDEX"                 { return TOKEN(INDEX);          }
"AND"                   { return TOKEN(AND);            }
"<>"                    { return TOKEN(NEQ);            }
"<="                    { return TOKEN(LE);             }
">="                    { return TOKEN(GE);             }
"="                     { return TOKEN(EQ);             }
"<"                     { return TOKEN(LT);             }
">"                     { return TOKEN(GT);             }

{NEWLINE}               { /*skip*/ }
{WHITESPACE}            { /*skip*/ }
{IDENTIFIER}            { SAVE_TOKEN; return TOKEN(IDENTIFIER);     }
{IDENTIFIER2}           { SAVE_TOKEN2; return TOKEN(IDENTIFIER);     }
{VALUE_INT}             { SAVE_TOKEN; return TOKEN(VALUE_INT);      }
{VALUE_STRING1}         { SAVE_TOKEN2; return TOKEN(VALUE_STRING);   }
{VALUE_STRING2}         { SAVE_TOKEN2; return TOKEN(VALUE_STRING);   }
{SIMPLE_OPERATOR}       { return yytext[0];                         }

.                       { printf("Unknown token [%c] at line %d!\n", yytext[0], yylineno + 1); yyterminate(); }

%%