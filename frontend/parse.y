%{

#include <iostream>
#include <cstdio>
#include <string>

#include "stmts.hpp"
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
    Value* value;
    ValueList* value_list;
    ValueLists* value_lists;
    Col* col;
    Expr* expr;
    WhereClause* where_clause;
    WhereClauses* where_clauses;
    SetClause* set_clause;
    SetClauses* set_clauses;
    Selector* selector;
    ColSelector* col_selector;
    TableList* table_list;
}

%token DATABASE DATABASES TABLE TABLES
%token SHOW CREATE DROP USE PRIMARY
%token KEY NOT TNULL INSERT INTO VALUES
%token DELETE FROM WHERE UPDATE
%token SET SELECT IS PINT VARCHAR DESC INDEX AND
%token DATE FLOAT FOREIGN REFERENCES
%token IDENTIFIER VALUE_INT VALUE_STRING
%token NEQ LE GE EQ LT GT

%type <str> dbName tbName colName IDENTIFIER VALUE_STRING
%type <int_value> VALUE_INT op
%type <pro> program
%type <stmt> stmt sysStmt dbStmt tbStmt idxStmt
%type <field> field
%type <type> type
%type <fieldList> fieldList
%type <value> value
%type <value_list> valueList
%type <value_lists> valueLists
%type <col> col
%type <expr> expr
%type <where_clause> whereClause
%type <where_clauses> whereClauses
%type <set_clause> setClause
%type <set_clauses> setClauses
%type <selector> selector
%type <col_selector> colSelector
%type <table_list> tableList

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
                        $$ = new DescTable($2);
                    }
                | INSERT INTO tbName VALUES valueLists
                    {
                        $$ = new InsertStatement($3, $5);
                    }
                | DELETE FROM tbName WHERE whereClauses
                    {
                        $$ = new DeleteStatement($3, $5);
                    }
                | UPDATE tbName SET setClauses WHERE whereClauses
                    {
                        $$ = new UpdateStatement($2, $4, $6);
                    }
                | SELECT selector FROM tableList WHERE whereClauses
                    {
                        $$ = new SelectStatement($2, $4, $6);
                    }
                ;

idxStmt         : CREATE INDEX tbName '(' colName ')'
                    {
                        $$ = new CreateIndexStatement($3, $5);
                    }
                | DROP INDEX tbName '(' colName ')'
                    {
                        $$ = new CreateIndexStatement($3, $5);
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
                        $$ = new Type(INT_ENUM, $3);
                    }
                | VARCHAR '(' VALUE_INT ')'
                    {
                        $$ = new Type(VARCHAR_ENUM, $3);
                    }
                | DATE
                    {
                        $$ = new Type(DATE_ENUM);
                    }
                | FLOAT
                    {
                        $$ = new Type(FLOAT_ENUM);
                    }
                ;

valueLists      : '(' valueList ')'
                    {
                        $$ = new ValueLists();
                        $$->lists.push_back($2);
                    }
                | valueLists ',' '(' valueList ')'
                    {
                        $$ = $1;
                        $$->lists.push_back($4);
                    }
                ;

valueList       : value
                    {
                        $$ = new ValueList();
                        $$->values.push_back($1);
                    }
                | valueList ',' value
                    {
                        $$ = $1;
                        $$->values.push_back($3);
                    }
                ;

value           : VALUE_INT
                    {
                        $$ = new Value($1);
                    }
                | VALUE_STRING
                    {
                        $$ = new Value($1);
                    }
                | TNULL
                    {
                        $$ = new Value();
                    }
                ;

whereClauses    : whereClause
                    {
                        $$ = new WhereClauses();
                        $$->clauses.push_back($1);
                    }
                | whereClauses AND whereClause
                    {
                        $$ = $1;
                        $$->clauses.push_back($3);
                    }
                ;

whereClause     : col op expr
                    {
                        $$ = new WhereClause($1, $2, $3);
                    }
                | col IS TNULL
                    {
                        $$ = new WhereClause($1, OP_IS_NULL);
                    }
                | col IS NOT TNULL
                    {
                        $$ = new WhereClause($1, OP_NOT_NULL);
                    }
                ;

col             : colName
                    {
                        $$ = new Col($1);
                    }
                | tbName '.' colName
                    {
                        $$ = new Col($1, $3);
                    }
                ;

op              : EQ
                    {
                        $$ = OP_EQ;
                    }
                | NEQ
                    {
                        $$ = OP_NEQ;
                    }
                | LE
                    {
                        $$ = OP_LE;
                    }
                | GE
                    {
                        $$ = OP_GE;
                    }
                | LT
                    {
                        $$ = OP_LT;
                    }
                | GT
                    {
                        $$ = OP_GT;
                    }
                ;

expr            : value
                    {
                        $$ = new Expr($1);
                    }
                | col
                    {
                        $$ = new Expr($1);
                    }
                ;

setClauses      : setClause
                    {
                        $$ = new SetClauses();
                        $$->clauses.push_back($1);
                    }
                | setClauses ',' setClause
                    {
                        $$ = $1;
                        $$->clauses.push_back($3);
                    }
                ;

setClause       : colName EQ value
                    {
                        $$ = new SetClause($1, $3);
                    }
                ;

selector        : '*'
                    {
                        $$ = new Selector();
                    }
                | colSelector
                    {
                        $$ = new Selector($1);
                    }
                ;

colSelector     : col
                    {
                        $$ = new ColSelector();
                        $$->cols.push_back($1);
                    }
                | colSelector ',' col
                    {
                        $$ = $1;
                        $$->cols.push_back($3);
                    }
                ;

tableList       : tbName
                    {
                        $$ = new TableList();
                        $$->tables.push_back($1);
                    }
                | tableList ',' tbName
                    {
                        $$ = $1;
                        $$->tables.push_back($3);
                    }
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