#include "engine.h"
#include <disk/common.h>

void init_context(Context* ctx)
{
    ctx->pwd = get_cwd();
    ctx->storage_path = path_join(ctx->pwd, "database");
    rmdir(ctx->storage_path); // FIXME
    mkdirp(ctx->storage_path);
}
