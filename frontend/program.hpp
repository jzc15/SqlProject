#ifndef FRONTEND_PROGRAM_H
#define FRONTEND_PROGRAM_H

#include "stmts_base.hpp"
#include <engine/engine.h>
#include <iostream>

using namespace std;

class Program
{
public:
    Context ctx;
    vector<Statement*> stmts;

    ~Program()
    {
        for(auto x: stmts)
        {
            delete x;
        }
    }
    
    void run()
    {
        init_context(&ctx);

        for(auto& s: stmts)
        {
            s->run(&ctx);
        }
    }
};

#endif // FRONTEND_PROGRAM_H