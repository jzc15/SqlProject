#include "dbop.h"

void show_databases(Context* ctx)
{
    TITLE(show_databases);
    vector<string> databases = listdir(ctx->storage_path);
    for(int i = 0; i < (int)databases.size(); i ++)
        cout << databases[i] << endl;
}

void create_database(Context* ctx, const string& db_name)
{
    mkdirp(path_join(ctx->storage_path, db_name));
}

void drop_database(Context* ctx, const string& db_name)
{
    rmdir(path_join(ctx->storage_path, db_name));
}

void use_database(Context* ctx, const string& db_name)
{
    ctx->current_database = db_name;
    ctx->dd = make_shared<DBDesc>(db_name, path_join(ctx->storage_path, db_name));
}
