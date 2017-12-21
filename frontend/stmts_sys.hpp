#ifndef FRONTEND_STMTS_SYS_H
#define FRONTEND_STMTS_SYS_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>

#include "stmts_base.hpp"

using namespace std;

class ShowDatabases : public Statement
{
public:
    void run(Context* ctx)
    {
        show_databases(ctx);
    }
};

#endif // FRONTEND_STMTS_SYS_H