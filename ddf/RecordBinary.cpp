#include "RecordBinary.h"

const RecordBinary nullrecord = (RecordBinary){NULL, 0};

bool operator ==(const RecordBinary& A, const RecordBinary& B)
{
    return A.ptr == B.ptr && A.size == B.size;
}

bool operator <(const RecordBinary& A, const RecordBinary& B)
{
    uint size = A.size;
    if(B.size < size)
        size = B.size;
    for(uint i = 0; i < B.size; i++)
        if(A.ptr[i] != B.ptr[i])
            return A.ptr[i] < B.ptr[i];
    return A.size < B.size;
}