#ifndef ENGINE_DB_OP_H
#define ENGINE_DB_OP_H

#include "context.h"
#include "helper.h"

void show_databases(Context* ctx);
void create_database(Context* ctx, const string& db_name);
void drop_database(Context* ctx, const string& db_name);
void use_database(Context* ctx, const string& db_name);

#endif // ENGINE_DB_OP_H