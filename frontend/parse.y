%{

#include <iostream>
#include <cstdio>
#include <string>
#include <vector>

#include "stmts.hpp"
#include "helper.h"

using namespace std;

// #define DELETE(x) /*nothing*/
#define DELETE(x) {delete x;}

%}

%union {
    std::string* str;
    int token;
    Statement* stmt;
    vector<CreateTable::Field>* fields;
    CreateTable::Field* field;
    Type* type;
    Value* value;
    vector<Value>* values;
    vector<vector<Value>>* values_list;
    Column* column;
    Expr* expr;
    Condition* cond;
    vector<Condition>* conds;
    Condition::OP op;
    Assignment* assignment;
    vector<Assignment>* assignments;
    Selector* selector;
    vector<Column>* columns;
    vector<string>* strings;
}

%token DATABASE DATABASES TABLE TABLES
%token SHOW CREATE DROP USE PRIMARY
%token KEY NOT TNULL INSERT INTO VALUES
%token DELETE FROM WHERE UPDATE
%token SET SELECT IS PINT VARCHAR DESC INDEX AND
%token DATE FLOAT FOREIGN REFERENCES DECIMAL
%token IDENTIFIER VALUE_INT VALUE_STRING VALUE_FLOAT
%token NEQ LE GE EQ LT GT

%type <str> dbName tbName colName IDENTIFIER VALUE_STRING VALUE_INT VALUE_FLOAT
%type <pro> program
%type <stmt> stmt sysStmt dbStmt tbStmt idxStmt
%type <field> field
%type <type> type
%type <fields> fieldList
%type <value> value
%type <values> valueList
%type <values_list> valueLists
%type <column> col
%type <expr> expr
%type <cond> whereClause
%type <conds> whereClauses
%type <op> op
%type <assignment> setClause
%type <assignments> setClauses
%type <selector> selector
%type <columns> colSelector
%type <strings> tableList columnList

%start program

%%

program         : /* empty */
                    {
                        
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
                        $$ = new CreateDatabase(*$3);
                        DELETE($3);
                    }
                | DROP DATABASE dbName
                    {
                        $$ = new DropDatabase(*$3);
                        DELETE($3);
                    }
                | USE dbName
                    {
                        $$ = new UseDatabase(*$2);
                        DELETE($2);
                    }
                | SHOW TABLES
                    {
                        $$ = new ShowTables();
                    }
                ;

tbStmt          : CREATE TABLE tbName '(' fieldList ')'
                    {
                        $$ = new CreateTable(*$3, *$5);
                        DELETE($3);
                        DELETE($5);
                    }
                | DROP TABLE tbName
                    {
                        $$ = new DropTable(*$3);
                        DELETE($3);
                    }
                | DESC tbName
                    {
                        $$ = new DescTable(*$2);
                        DELETE($2);
                    }
                | INSERT INTO tbName VALUES valueLists
                    {
                        $$ = new InsertStatement(*$3, *$5);
                        DELETE($3);
                        DELETE($5);
                    }
                | DELETE FROM tbName WHERE whereClauses
                    {
                        $$ = new DeleteStatement(*$3, *$5);
                        DELETE($3);
                        DELETE($5);
                    }
                | UPDATE tbName SET setClauses WHERE whereClauses
                    {
                        $$ = new UpdateStatement(*$2, *$4, *$6);
                        DELETE($2);
                        DELETE($4);
                        DELETE($6);
                    }
                | SELECT selector FROM tableList WHERE whereClauses
                    {
                        $$ = new SelectStatement(*$2, *$4, *$6);
                        DELETE($2);
                        DELETE($4);
                        DELETE($6);
                    }
                ;

idxStmt         : CREATE INDEX tbName '(' colName ')'
                    {
                        $$ = new CreateIndexStatement(*$3, *$5);
                        DELETE($3);
                        DELETE($5);
                    }
                | DROP INDEX tbName '(' colName ')'
                    {
                        $$ = new DropIndexStatement(*$3, *$5);
                        DELETE($3);
                        DELETE($5);
                    }
                ;

fieldList       : field
                    {
                        $$ = new vector<CreateTable::Field>();
                        $$->push_back(*$1);
                        DELETE($1);
                    }
                | fieldList ',' field
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        DELETE($3);
                    }
                ;

field           : colName type
                    {
                        $$ = new CreateTable::Field();
                        $$->type_no = 1;
                        $$->column_define = ColumnDefine(*$1, *$2, true);
                        DELETE($1);
                        DELETE($2);
                    }
                | type type
                    {
                        cerr << "[warning] keyword `" << type_name($1->type) << "` as a column name." << endl;
                        $$ = new CreateTable::Field();
                        $$->type_no = 1;
                        $$->column_define = ColumnDefine(type_name($1->type), *$2, true);
                        DELETE($1);
                        DELETE($2);
                    }
                | colName type NOT TNULL
                    {
                        $$ = new CreateTable::Field();
                        $$->type_no = 2;
                        $$->column_define = ColumnDefine(*$1, *$2, false);
                        DELETE($1);
                        DELETE($2);
                    }
                | PRIMARY KEY '(' columnList ')'
                    {
                        $$ = new CreateTable::Field();
                        $$->type_no = 3;
                        $$->primary_columns = *$4;
                        DELETE($4);
                    }
                | FOREIGN KEY '(' colName ')' REFERENCES tbName '(' colName ')'
                    {
                        $$ = new CreateTable::Field();
                        $$->type_no = 4;
                        $$->foreign_column = ColumnForeign(*$4, *$7, *$9);
                        DELETE($4);
                        DELETE($7);
                        DELETE($9);
                    }
                ;

columnList      : colName
                {
                    $$ = new vector<string>();
                    $$->push_back(*$1);
                    DELETE($1);
                }
                | columnList ',' colName
                {
                    $$ = $1;
                    $$->push_back(*$3);
                    DELETE($3);
                }
                ;

type            : PINT '(' VALUE_INT ')'
                    {
                        $$ = new Type(INT_ENUM, atoi($3->c_str()));
                        DELETE($3);
                    }
                | VARCHAR '(' VALUE_INT ')'
                    {
                        $$ = new Type(VARCHAR_ENUM, atoi($3->c_str()));
                        DELETE($3);
                    }
                | DATE
                    {
                        $$ = new Type(DATE_ENUM);
                    }
                | FLOAT
                    {
                        $$ = new Type(FLOAT_ENUM);
                    }
                | DECIMAL '(' VALUE_INT ',' VALUE_INT ')'
                    {
                        $$ = new Type(DECIMAL_ENUM);
                    }
                ;

valueLists      : '(' valueList ')'
                    {
                        $$ = new vector<vector<Value>>();
                        $$->push_back(*$2);
                        DELETE($2);
                    }
                | valueLists ',' '(' valueList ')'
                    {
                        $$ = $1;
                        $$->push_back(*$4);
                        DELETE($4);
                    }
                ;

valueList       : value
                    {
                        $$ = new vector<Value>();
                        $$->push_back(*$1);
                        DELETE($1);
                    }
                | valueList ',' value
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        DELETE($3);
                    }
                ;

value           : VALUE_INT
                    {
                        $$ = new Value();
                        *$$ = Value::int_value(*$1);
                        DELETE($1);
                    }
                | VALUE_STRING
                    {
                        $$ = new Value();
                        *$$ = Value::string_value(*$1);
                        DELETE($1);
                    }
                | VALUE_FLOAT
                    {
                        $$ = new Value();
                        *$$ = Value::float_value(*$1);
                        DELETE($1);
                    }
                | TNULL
                    {
                        $$ = new Value();
                        *$$ = Value::null_value();
                    }
                ;

whereClauses    : whereClause
                    {
                        $$ = new vector<Condition>();
                        $$->push_back(*$1);
                        DELETE($1);
                    }
                | whereClauses AND whereClause
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        DELETE($3);
                    }
                ;

whereClause     : col op expr
                    {
                        $$ = new Condition();
                        *$$ = Condition::expr_condition(*$1, $2, *$3);
                        DELETE($1);
                        DELETE($3);
                    }
                | col IS TNULL
                    {
                        $$ = new Condition();
                        *$$ = Condition::is_null_condition(*$1);
                        DELETE($1);
                    }
                | col IS NOT TNULL
                    {
                        $$ = new Condition();
                        *$$ = Condition::not_null_condition(*$1);
                        DELETE($1);
                    }
                ;

col             : colName
                    {
                        $$ = new Column();
                        *$$ = Column::create_column(*$1);
                        DELETE($1);
                    }
                | tbName '.' colName
                    {
                        $$ = new Column();
                        *$$ = Column::create_column(*$1, *$3);
                        DELETE($1);
                        DELETE($3);
                    }
                ;

op              : EQ
                    {
                        $$ = Condition::OP_EQ;
                    }
                | NEQ
                    {
                        $$ = Condition::OP_NEQ;
                    }
                | LE
                    {
                        $$ = Condition::OP_LE;
                    }
                | GE
                    {
                        $$ = Condition::OP_GE;
                    }
                | LT
                    {
                        $$ = Condition::OP_LT;
                    }
                | GT
                    {
                        $$ = Condition::OP_GT;
                    }
                ;

expr            : value
                    {
                        $$ = new Expr();
                        *$$ = Expr::value_expr(*$1);
                        DELETE($1);
                    }
                | col
                    {
                        $$ = new Expr();
                        *$$ = Expr::column_expr(*$1);
                        DELETE($1);
                    }
                ;

setClauses      : setClause
                    {
                        $$ = new vector<Assignment>();
                        $$->push_back(*$1);
                        DELETE($1);
                    }
                | setClauses ',' setClause
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        DELETE($3);
                    }
                ;

setClause       : colName EQ value
                    {
                        $$ = new Assignment();
                        *$$ = Assignment::assign(*$1, *$3);
                        DELETE($1);
                        DELETE($3);
                    }
                ;

selector        : '*'
                    {
                        $$ = new Selector();
                        *$$ = Selector::all_selector();
                    }
                | colSelector
                    {
                        $$ = new Selector();
                        *$$ = Selector::columns_selector(*$1);
                        DELETE($1);
                    }
                ;

colSelector     : col
                    {
                        $$ = new vector<Column>();
                        $$->push_back(*$1);
                        DELETE($1);
                    }
                | colSelector ',' col
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        DELETE($3);
                    }
                ;

tableList       : tbName
                    {
                        $$ = new vector<string>();
                        $$->push_back(*$1);
                        DELETE($1);
                    }
                | tableList ',' tbName
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        DELETE($3);
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