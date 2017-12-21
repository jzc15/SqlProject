#ifndef FRONTEND_STMTS_IDX_H
#define FRONTEND_STMTS_IDX_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "stmts_base.hpp"

using namespace std;

class CreateIndexStatement : public Statement
{
public:
    string tb_name;
    string column_name;
    CreateIndexStatement(const string& tb_name, const string& column_name) : tb_name(tb_name), column_name(column_name) {}

    void run(Context* ctx)
    {
        create_index(ctx, tb_name, column_name);
    }
};

class DropIndexStatement : public Statement
{
public:
    string tb_name;
    string column_name;
    DropIndexStatement(const string& tb_name, const string& column_name) : tb_name(tb_name), column_name(column_name) {}

    void run(Context* ctx)
    {
        drop_index(ctx, tb_name, column_name);
    }
};

#endif // FRONTEND_STMTS_IDX_H