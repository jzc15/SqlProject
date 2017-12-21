#ifndef FRONTEND_CONTEXT_H
#define FRONTEND_CONTEXT_H

#include <string>
#include <ddf/dbdesc.h>

using namespace std;

struct Context // 上下文
{
    string pwd;
    string storage_path;
    string current_database;
    DBDesc::ptr dd;
};

#endif // FRONTEND_CONTEXT_H