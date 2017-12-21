#ifndef FRONTEND_STMTS_DB_H
#define FRONTEND_STMTS_DB_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "stmts_base.hpp"

using namespace std;

class CreateDatabase : public Statement
{
public:
    string db_name;
    CreateDatabase(const string& db_name): db_name(db_name) { }

    void run(Context* ctx)
    {
        create_database(ctx, db_name);
    }
};

class DropDatabase : public Statement
{
public:
    string db_name;
    DropDatabase(const string& db_name): db_name(db_name) {}

    void run(Context* ctx)
    {
        drop_database(ctx, db_name);
    }
};

class UseDatabase : public Statement
{
public:
    string db_name;
    UseDatabase(const string& db_name): db_name(db_name) {}

    void run(Context* ctx)
    {
        use_database(ctx, db_name);
    }
};

class ShowTables : public Statement
{
public:
    void run(Context* ctx)
    {
        show_tables(ctx);
    }
};

#endif // FRONTEND_STMTS_DB_H