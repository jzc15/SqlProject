#ifndef FRONTEND_PROGRAM_H
#define FRONTEND_PROGRAM_H

#include "stmts_base.hpp"
#include <disk/common.h>
#include <iostream>

using namespace std;

class Program
{
public:
    Context ctx;
    vector<Statement*> stmts;
    
    void run()
    {
        ctx.pwd = get_cwd();
        ctx.storage_path = path_join(ctx.pwd, "database");
        rmdir(ctx.storage_path); // FIXME
        mkdirp(ctx.storage_path);

        for(auto s: stmts)
        {
            s->run(&ctx);
        }
    }
};

#endif // FRONTEND_PROGRAM_H