#ifndef _RECORD_BINARY_H
#define _RECORD_BINARY_H

#include "filesystem/utils/pagedef.h"

// 记录二进制
struct RecordBinary
{
    uchar* ptr;
    ushort size;
};

extern const RecordBinary nullrecord; // 用于表示空记录/无记录

// 判断相等
bool operator ==(const RecordBinary& A, const RecordBinary& B);

#endif // _RECORD_BINARY_H