#ifndef FRONTEND_CONTEXT_H
#define FRONTEND_CONTEXT_H

#include <string>
#include <ddf/DatabaseDescription.h>

using namespace std;

struct Context // 上下文
{
    string pwd;
    string current_database;
    DatabaseDescription::ptr dd;
};

#endif // FRONTEND_CONTEXT_H