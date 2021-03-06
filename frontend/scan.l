
%{

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "stmts.hpp"
#include "frontend_parse.h"
#include "helper.h"

using namespace std;

#define SAVE_TOKEN yylval.str = new std::string(yytext, yyleng);
#define SAVE_TOKEN2 yylval.str = new std::string(yytext+1, yyleng-2)
#define TOKEN(t) yylval.token = t
#define PRINT(t) { \
    if (scan_debug) printf("[%s] %s\n", std::string(yytext, yyleng).c_str(), #t); \
}


#ifndef PRINT
#define PRINT(t) { /*nothing*/ }
#endif

%}

%option noyywrap
%option yylineno

%x line_comment
%x block_comment

NEWLINE                 (\r|\n|\r\n)
WHITESPACE			    ([ \t]+)
IDENTIFIER              ([A-Za-z][_0-9A-Za-z]*)
IDENTIFIER2             `([A-Za-z][_0-9A-Za-z]*)`
VALUE_INT               (-?[0-9]+)
VALUE_STRING            '([^\'\n]|(\\\'))*'
VALUE_FLOAT             (-?[0-9]+\.[0-9]+)
SIMPLE_OPERATOR         (";"|"`"|"'"|"("|")"|","|"*"|".")

%%

"--"                    { BEGIN(line_comment);                          }
<line_comment>[^\r\n] {}
<line_comment>{NEWLINE} { BEGIN(INITIAL);                               }
"/*"                    { BEGIN(block_comment);                         }
<block_comment>[^*]     { /* ignore the text of the comment */          }
<block_comment>"*/"     { BEGIN(INITIAL);                               }
<block_comment>\*       { /* ignore *'s that aren't part of */          }

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
"DECIMAL"               { PRINT(DECIMAL); return TOKEN(DECIMAL);        }
"FOREIGN"               { PRINT(FOREIGN); return TOKEN(FOREIGN);        }
"REFERENCES"            { PRINT(REFERENCES); return TOKEN(REFERENCES);  }
"<>"                    { PRINT(NEQ); return TOKEN(NEQ);                }
"<="                    { PRINT(LE); return TOKEN(LE);                  }
">="                    { PRINT(GE); return TOKEN(GE);                  }
"="                     { PRINT(EQ); return TOKEN(EQ);                  }
"<"                     { PRINT(LT); return TOKEN(LT);                  }
">"                     { PRINT(GT); return TOKEN(GT);                  }
"LIKE"                  { PRINT(LIKE); return TOKEN(LIKE);              }
"AVG"                   { PRINT(AVG); return TOKEN(AVG);                }
"SUM"                   { PRINT(SUM); return TOKEN(SUM);                }
"MIN"                   { PRINT(MIN); return TOKEN(MIN);                }
"MAX"                   { PRINT(MAX); return TOKEN(MAX);                }
"COUNT"                 { PRINT(COUNT); return TOKEN(COUNT);            }
"CHECK"                 { PRINT(CHECK); return TOKEN(CHECK);            }
"IN"                    { PRINT(IN); return TOKEN(IN);                  }

{NEWLINE}               { /*skip*/                                                      }
{WHITESPACE}            { /*skip*/                                                      }
{IDENTIFIER}            { SAVE_TOKEN; PRINT(IDENTIFIER); return IDENTIFIER;             }
{IDENTIFIER2}           { SAVE_TOKEN2; PRINT(IDENTIFIER); return IDENTIFIER;            }
{VALUE_FLOAT}           { SAVE_TOKEN; PRINT(VALUE_FLOAT); return VALUE_FLOAT;           }
{VALUE_INT}             { SAVE_TOKEN; PRINT(VALUE_INT); return VALUE_INT;               }
{VALUE_STRING}          { SAVE_TOKEN2; PRINT(VALUE_STRING); return VALUE_STRING;        }
{SIMPLE_OPERATOR}       { return yytext[0];                                             }

.                       { printf("Unknown token [%c] at line %d!\n", yytext[0], yylineno); yyterminate(); exit(1); }

%%