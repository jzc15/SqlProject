%{

#include <iostream>
#include <cstdio>
#include <string>

#include "nodes.hpp"
#include "helper.h"

using namespace std;

%}

%union {
    std::string* str;
    int int_value;
    int token;
    Statement* stmt;
    Field* field;
    Type* type;
    FieldList* fieldList;
}

%token DATABASE DATABASES TABLE TABLES
%token SHOW CREATE DROP USE PRIMARY
%token KEY NOT TNULL INSERT INTO VALUES
%token DELETE FROM WHERE UPDATE
%token SET SELECT IS PINT VARCHAR DESC INDEX AND
%token DATE FLOAT FOREIGN REFERENCES
%token IDENTIFIER VALUE_INT VALUE_STRING
%token NEQ LE GE EQ LT GT

%type <str> dbName tbName colName IDENTIFIER
%type <int_value> VALUE_INT
%type <pro> program
%type <stmt> stmt sysStmt dbStmt tbStmt idxStmt
%type <field> field
%type <type> type
%type <fieldList> fieldList

%start program

%%

program         : /* empty */
                    {
                        pro = new Program();
                    }
                | program stmt
                    {
                        pro->stmts.push_back($2);
                    }
                ;

stmt            : sysStmt ';'
                    {
                        $$ = $1;
                    }
                | dbStmt ';'
                    {
                        $$ = $1;
                    }
                | tbStmt ';'
                    {
                        $$ = $1;
                    }
                | idxStmt ';'
                    {
                        $$ = $1;
                    }
                ;

sysStmt         : SHOW DATABASES
                    {
                        $$ = new ShowDatabases();
                    }
                ;

dbStmt          : CREATE DATABASE dbName
                    {
                        $$ = new CreateDatabase($3);
                    }
                | DROP DATABASE dbName
                    {
                        $$ = new DropDatabase($3);
                    }
                | USE dbName
                    {
                        $$ = new UseDatabase($2);
                    }
                | SHOW TABLES
                    {
                        $$ = new ShowTables();
                    }
                ;

tbStmt          : CREATE TABLE tbName '(' fieldList ')'
                    {
                        $$ = new CreateTable($3, $5);
                    }
                | DROP TABLE tbName
                    {
                        $$ = new DropTable($3);
                    }
                | DESC tbName
                    {
                        $$ = new unStmt();
                    }
                | INSERT INTO tbName VALUES valueLists
                    {
                        $$ = new unStmt();
                    }
                | DELETE FROM tbName WHERE whereClause
                    {
                        $$ = new unStmt();
                    }
                | UPDATE tbName SET setClause WHERE whereClause
                    {
                        $$ = new unStmt();
                    }
                | SELECT selector FROM tableList WHERE whereClause
                    {
                        $$ = new unStmt();
                    }
                ;

idxStmt         : CREATE INDEX tbName '(' colName ')'
                    {
                        $$ = new unStmt();
                    }
                | DROP INDEX tbName '(' colName ')'
                    {
                        $$ = new unStmt();
                    }
                ;

fieldList       : field
                    {
                        $$ = new FieldList();
                        $$->fields.push_back($1);
                    }
                | fieldList ',' field
                    {
                        $$ = $1;
                        $$->fields.push_back($3);
                    }
                ;

field           : colName type
                    {
                        $$ = new Field($1, $2);
                    }
                | colName type NOT TNULL
                    {
                        $$ = new Field($1, $2, true);
                    }
                | PRIMARY KEY '(' colName ')'
                    {
                        $$ = new Field($4, true, NULL, NULL);
                    }
                | FOREIGN KEY '(' colName ')' REFERENCES tbName '(' colName ')'
                    {
                        $$ = new Field($4, false, $7, $9);
                    }
                ;

type            : PINT '(' VALUE_INT ')'
                    {
                        $$ = new Type(TYPE_INT, $3);
                    }
                | VARCHAR '(' VALUE_INT ')'
                    {
                        $$ = new Type(TYPE_VARCHAR, $3);
                    }
                | DATE
                    {
                        $$ = new Type(TYPE_DATE);
                    }
                | FLOAT
                    {
                        $$ = new Type(TYPE_FLOAT);
                    }
                ;

valueLists      : '(' valueList ')'
                | valueLists ',' '(' valueList ')'
                ;

valueList       : value
                | valueList ',' value
                ;

value           : VALUE_INT
                | VALUE_STRING
                | TNULL
                ;

whereClause     : col op expr
                | col IS TNULL
                | col IS NOT TNULL
                | whereClause AND whereClause
                ;

col             : colName
                | tbName '.' colName
                ;

op              : EQ
                | NEQ
                | LE
                | GE
                | LT
                | GT
                ;

expr            : value
                | col
                ;

setClause       : colName '=' value
                | setClause ',' colName '=' value
                ;

selector        : '*'
                | colSelector
                ;

colSelector     : col
                | colSelector ',' col
                ;

tableList       : tbName
                | tableList ',' tbName
                ;

dbName          : IDENTIFIER
                    {
                        $$ = $1;
                    }
                ;

tbName          : IDENTIFIER
                    {
                        $$ = $1;
                    }
                ;

colName         : IDENTIFIER
                    {
                        $$ = $1;
                    }
                ;

%%