#ifndef FRONTEND_STMTS_BASE_H
#define FRONTEND_STMTS_BASE_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "context.hpp"
#include <disk/common.h>
#include <datamanager/slotsfile.h>
#include <ddf/dbdesc.h>
#include <ddf/record.h>
#include <ddf/typeinfo.h>

using namespace std;

class Statement
{
public:
    virtual ~Statement() {}
    virtual void run(Context* ctx) = 0;
};

// nothing todo, should not be usedif possible
class unStmt : public Statement
{
public:
    unStmt() {}

    void run(Context* ctx)
    {
        cerr << "!!!!!!!!!!!!!!!!!TODO!!!!!!!!!!!!!!!!!" << endl;
    }
};

#endif // FRONTEND_STMTS_BASE_H