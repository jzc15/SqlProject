#ifndef FRONTEND_NODES_H
#define FRONTEND_NODES_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "const.hpp"
#include "context.hpp"

#include <ddf/DatabaseDescription.h>

using namespace std;

class Statement
{
public:
    virtual ~Statement() {}
    virtual void run(Context* ctx) = 0;
};

class ShowDatabases : public Statement
{
public:
    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

class CreateDatabase : public Statement
{
public:
    string* name;
    CreateDatabase(string* name): name(name) { }

    void run(Context* ctx)
    {
        DatabaseDescription dd(*name);
        dd.Save();
    }
};

class DropDatabase : public Statement
{
public:
    string* name;
    DropDatabase(string* name): name(name) {}

    void run(Context* ctx)
    {
        if (*name == ctx->current_database)
        {
            ctx->current_database = "";
            ctx->dd = nullptr;
        }

        char buf[1024];
        sprintf(buf, "rm -rf %s", name->c_str());
        system(buf);
    }
};

class UseDatabase : public Statement
{
public:
    string* name;
    UseDatabase(string* name): name(name) {}

    void run(Context* ctx)
    {
        ctx->current_database = *name;
        ctx->dd = make_shared<DatabaseDescription>(*name);
    }
};

class ShowTables : public Statement
{
public:
    void run(Context* ctx)
    {
        vector<string> tables = ctx->dd->TableList();
        for(auto table: tables)
        {
            cout << table << endl;
        }
    }
};

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

class unStmt : public Statement
{
public:
    unStmt() {}

    void run(Context* ctx)
    {
        cerr << "!!!!!!!!!!!!!!!!!TODO!!!!!!!!!!!!!!!!!" << endl;
    }
};

#endif // FRONTEND_NODES_H