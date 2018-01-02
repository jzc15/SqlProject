#include "dbop.h"
#include "helper.h"

void show_databases(Context* ctx)
{
    TITLE(show_databases);
    if (debug_on)
    {
        *out << ctx->pwd << endl;
        *out << ctx->storage_path << endl;
    }

    vector<string> databases = listdir(ctx->storage_path);
    for(int i = 0; i < (int)databases.size(); i ++)
        *out << databases[i] << endl;
}

void create_database(Context* ctx, const string& db_name)
{
    TITLE(create_database)
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
