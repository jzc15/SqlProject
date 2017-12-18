#ifndef FRONTEND_STMTS_SYS_H
#define FRONTEND_STMTS_SYS_H

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

class ShowDatabases : public Statement
{
public:
    void run(Context* ctx)
    {
        cerr << "TODO" << endl;
    }
};

#endif // FRONTEND_STMTS_SYS_H