#ifndef FRONTEND_STMTS_TB_H
#define FRONTEND_STMTS_TB_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "const.hpp"
#include "context.hpp"
#include "stmts_base.hpp"

#include <ddf/DatabaseDescription.h>

using namespace std;

class Type
{
public:
    FLAG type;
    int limit;

    Type(FLAG type, int limit = 0): type(type), limit(limit) {}
    string typeName()
    {
        switch(type)
        {
            case TYPE_INT: return "int";
            case TYPE_VARCHAR: return "varchar";
            case TYPE_DATE: return "date";
            case TYPE_FLOAT: return "float";
            default: assert(false);
        }
    }
};

class Field // 定义列/描述列
{
public:
    bool new_col;

    string* colName;
    Type* type;
    bool not_null;

    bool is_key;
    string* ref_tbName;
    string* ref_colName;

    Field(string* colName, Type* type, bool not_null = false)
        : new_col(true), colName(colName), type(type), not_null(not_null)
        {

        }
    Field(string* colName, bool is_key, string* ref_tbName, string* ref_colName)
        : new_col(false), colName(colName), is_key(is_key), ref_tbName(ref_tbName), ref_colName(ref_colName)
        {

        }
};

class FieldList
{
public:
    vector<Field*> fields;
};

class CreateTable : public Statement
{
public:
    string* tbName;
    FieldList* fieldList;

    CreateTable(string* tbName, FieldList* fieldList): tbName(tbName), fieldList(fieldList) {}

    void run(Context* ctx)
    {
        TableDescription::ptr td = ctx->dd->CreateTable(*tbName);
        for(auto field: fieldList->fields)
        {
            if (field->new_col)
            {
                td->CreateColumn(*(field->colName), field->type->typeName(), (field->type->typeName() == "varchar") * field->type->limit); // FIXME
            }
        }
        td->Finalize();
    }
};

class DropTable : public Statement
{
public:
    string* tbName;

    DropTable(string* tbName): tbName(tbName) {}

    void run(Context* ctx)
    {
        ctx->dd->DropTable(*tbName);
    }
};

class DescTable : public Statement
{
public:
    string* tbName;

    DescTable(string* tbName): tbName(tbName) {}

    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

class Value
{
public:
    enum value_t { VALUE_INT, VALUE_STRING, VALUE_NULL };
    value_t value_type;
    int int_value;
    string* string_value;

    Value(): value_type(VALUE_NULL) {}
    Value(int int_value): value_type(VALUE_INT), int_value(int_value) {}
    Value(string* string_value): value_type(VALUE_STRING), string_value(string_value) {}
};

class ValueList
{
public:
    vector<Value*> values;
};
class ValueLists
{
public:
    vector<ValueList*> lists;
};

class InsertStatement : public Statement
{
public:
    string* tbName;
    ValueLists* lists;

    InsertStatement(string* tbName, ValueLists* lists): tbName(tbName), lists(lists) {}

    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

#define OP_EQ 1
#define OP_NEQ 2
#define OP_LE 3
#define OP_GE 4
#define OP_LT 5
#define OP_GT 6
#define OP_IS_NULL 7
#define OP_NOT_NULL 8

class Col
{
public:
    string* tbName;
    string* colName;
    Col(string* colName): tbName(NULL), colName(colName) {}
    Col(string* tbName, string* colName): tbName(tbName), colName(colName) {}
};

class Expr
{
public:
    Value* value;
    Col* col;
    Expr(Value* value): value(value), col(NULL) {}
    Expr(Col* col): value(NULL), col(col) {}
};

class WhereClause
{
public:
    Col* col;
    int op;
    Expr* expr;
    WhereClause(Col* col, int op, Expr* expr = NULL): col(col), op(op), expr(expr) {}
};

class WhereClauses
{
public:
    vector<WhereClause*> clauses;
};

class DeleteStatement : public Statement
{
public:
    string* tbName;
    WhereClauses* where;
    DeleteStatement(string* tbName, WhereClauses* where): tbName(tbName), where(where) {}

    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

class SetClause
{
public:
    string* colName;
    Value* value;
    SetClause(string* colName, Value* value): colName(colName), value(value) {}
};
class SetClauses
{
public:
    vector<SetClause*> clauses;
};

class UpdateStatement : public Statement
{
public:
    string* tbName;
    SetClauses* set;
    WhereClauses* where;
    UpdateStatement(string* tbName, SetClauses* set, WhereClauses* where): tbName(tbName), set(set), where(where) {}

    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

class ColSelector
{
public:
    vector<Col*> cols;
};

class Selector
{
public:
    ColSelector* col_selector;
    Selector(ColSelector* col_selector = NULL): col_selector(col_selector) {}
};

class TableList
{
public:
    vector<string*> tables;
};

class SelectStatement : public Statement
{
public:
    Selector* selector;
    TableList* table_list;
    WhereClauses* where;
    SelectStatement(Selector* selector, TableList* table_list, WhereClauses* where): selector(selector), table_list(table_list), where(where) {}

    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

#endif // FRONTEND_STMTS_TB_H