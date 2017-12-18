#ifndef FRONTEND_STMTS_IDX_H
#define FRONTEND_STMTS_IDX_H

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

class CreateIndexStatement : public Statement
{
public:
    string* tbName;
    string* colName;
    CreateIndexStatement(string* tbName, string* colName) : tbName(tbName), colName(colName) {}

    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

class DropIndexStatement : public Statement
{
public:
    string* tbName;
    string* colName;
    DropIndexStatement(string* tbName, string* colName) : tbName(tbName), colName(colName) {}

    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

#endif // FRONTEND_STMTS_IDX_H