#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "context.h"
#include "dbop.h"
#include "tableop.h"
#include "crudop.h"
#include "indexop.h"
#include "options.h"

void init_context(Context* ctx);

#endif // ENGINE_ENGINE_H