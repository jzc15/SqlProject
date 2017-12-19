#ifndef FRONTEND_STMTS_DB_H
#define FRONTEND_STMTS_DB_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "context.hpp"
#include "stmts_base.hpp"

using namespace std;

class CreateDatabase : public Statement
{
public:
    string* name;
    CreateDatabase(string* name): name(name) { }

    void run(Context* ctx)
    {
        mkdirp(path_join(ctx->storage_path, *name));
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

        rmdir(path_join(ctx->storage_path, *name));
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
        ctx->dd = make_shared<DBDesc>(*name, path_join(ctx->storage_path, *name));
    }
};

class ShowTables : public Statement
{
public:
    void run(Context* ctx)
    {
        vector<string> tables = ctx->dd->TableList();
        cout << "===========SHOW TABLES===========" << endl;
        for(auto table: tables)
        {
            cout << table << endl;
        }
    }
};

#endif // FRONTEND_STMTS_DB_H