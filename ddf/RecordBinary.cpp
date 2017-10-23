#include "RecordBinary.h"

const RecordBinary nullrecord = (RecordBinary){NULL, 0};

bool operator ==(const RecordBinary& A, const RecordBinary& B)
{
    return A.ptr == B.ptr && A.size == B.size;
}