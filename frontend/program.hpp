#ifndef FRONTEND_PROGRAM_H
#define FRONTEND_PROGRAM_H

#include "stmts_base.hpp"
#include <unistd.h>
#include <iostream>

using namespace std;

class Program
{
public:
    Context ctx;
    vector<Statement*> stmts;
    
    void run()
    {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        ctx.pwd = cwd;

        for(auto s: stmts)
        {
            s->run(&ctx);
        }
    }
};

#endif // FRONTEND_PROGRAM_H