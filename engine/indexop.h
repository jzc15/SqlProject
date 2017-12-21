#ifndef ENGING_INDEX_OP_H
#define ENGING_INDEX_OP_H

#include "context.h"
#include <string>

using namespace std;

void create_index(Context* ctx, const string& tb_name, const string& column_name);
void drop_index(Context* ctx, const string& tb_name, const string& column_name);

#endif // ENGING_INDEX_OP_H