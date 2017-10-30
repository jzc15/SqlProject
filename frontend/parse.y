%{

#include <iostream>
#include <cstdio>
#include <string>

#include "helper.h"

using namespace std;

%}

%union {
    std::string *str;
    int token;
}

%token DATABASE DATABASES TABLE TABLES
%token SHOW CREATE DROP USE PRIMARY
%token KEY NOT TNULL INSERT INTO VALUES
%token DELETE FROM WHERE UPDATE
%token SET SELECT IS INT VARCHAR DESC INDEX AND
%token IDENTIFIER VALUE_INT VALUE_STRING
%token NEQ LE GE EQ LT GT

%start program

%%

program         : /* empty */
                    {

                    }
                | program stmt
                    {

                    }
                ;

stmt            : sysStmt ';'
                | dbStmt ';'
                | tbStmt ';'
                | idxStmt ';'
                ;

sysStmt         : SHOW DATABASES
                ;

dbStmt          : CREATE DATABASE dbName
                | DROP DATABASE dbName
                | USE dbName
                | SHOW TABLES
                ;

tbStmt          : CREATE TABLE tbName '(' fieldList ')'
                | DROP TABLE tbName
                | DESC tbName
                | INSERT INTO tbName VALUES valueLists
                | DELETE FROM tbName WHERE whereClause
                | UPDATE tbName SET setClause WHERE whereClause
                | SELECT selector FROM tableList WHERE whereClause
                ;

idxStmt         : CREATE INDEX tbName '(' colName ')'
                | DROP INDEX tbName '(' colName ')'
                ;

fieldList       : field
                | fieldList ',' field
                ;

field           : colName type
                | colName type NOT TNULL
                | PRIMARY KEY '(' colName ')'
                ;

type            : INT '(' VALUE_INT ')'
                | VARCHAR '(' VALUE_INT ')'
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
                ;

tbName          : IDENTIFIER
                ;

colName         : IDENTIFIER
                ;

%%