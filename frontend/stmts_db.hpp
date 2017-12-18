#ifndef FRONTEND_STMTS_DB_H
#define FRONTEND_STMTS_DB_H

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

#endif // FRONTEND_STMTS_DB_H