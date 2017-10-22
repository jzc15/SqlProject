#ifndef _RECORD_BINARY_H
#define _RECORD_BINARY_H

#include "filesystem/utils/pagedef.h"

// 记录二进制
struct RecordBinary
{
    uchar* ptr;
    size_t size;
};

#endif // _RECORD_BINARY_H