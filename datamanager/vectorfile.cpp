#include "vectorfile.h"
#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

VectorFile::VectorFile(const string& filename)
    : filename(filename)
{
    file = make_shared<File>(filename);
    header = (header_t*)file->ReadPage(0)->data();
    if (!header->valid)
    {
        header->valid = 1;
        header->empty_pos = PAGE_SIZE;
    }

    assert(sizeof(vec_h_t)%sizeof(int) == 0);
}
VectorFile::~VectorFile()
{

}

int VectorFile::NewVector(int value)
{
    if (PAGE_SIZE - (header->empty_pos % PAGE_SIZE) < sizeof(vec_h_t))
        header->empty_pos = (header->empty_pos + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    const int pos = header->empty_pos;
    vec_h_t* vec_h = (vec_h_t*)(file->ReadPage(pos/PAGE_SIZE)->data() + pos%PAGE_SIZE);
    vec_h->size = 1;
    vec_h->capacity = 1;
    int data_pos = pos + sizeof(vec_h_t);
    *(int*)(file->ReadPage(data_pos/PAGE_SIZE)->data() + data_pos%PAGE_SIZE) = value;
    header->empty_pos += sizeof(vec_h_t) + sizeof(int);
    return pos;
}
vector_t VectorFile::Fetch(int pos)
{
    vec_h_t* vec_h = (vec_h_t*)(file->ReadPage(pos/PAGE_SIZE, true)->data() + pos%PAGE_SIZE);
    vector_t data = make_shared<vector<int>>();
    data->resize(vec_h->size);
    pos += sizeof(vec_h_t);
    for(int i = 0; i < vec_h->size; )
    {
        int len = min(vec_h->size - i, (PAGE_SIZE-(pos%PAGE_SIZE))/(int)sizeof(int));
        memcpy(data->data()+i, file->ReadPage(pos/PAGE_SIZE, true)->data()+(pos%PAGE_SIZE), len*sizeof(int));
        pos += len*sizeof(int);
        i += len;
    }
    return data;
}
int VectorFile::Save(int opos, vector_t data, bool append_only)
{
    vec_h_t* vec_h = (vec_h_t*)(file->ReadPage(opos/PAGE_SIZE, true)->data() + opos%PAGE_SIZE);
    int pos = opos; // 原位置

    if (vec_h->capacity < (int)data->size())
    {
        if (PAGE_SIZE - (header->empty_pos % PAGE_SIZE) < sizeof(vec_h_t))
            header->empty_pos = (header->empty_pos + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
        vec_h = (vec_h_t*)(file->ReadPage(header->empty_pos/PAGE_SIZE)->data() + header->empty_pos%PAGE_SIZE);
        vec_h->capacity = data->size() * 2;
        pos = header->empty_pos;
        header->empty_pos += sizeof(vec_h_t) + vec_h->capacity*sizeof(int);
        append_only = false;
    }
    int append_only_offset = vec_h->size;
    vec_h->size = data->size();

    const int npos = pos; // 新位置
    pos += sizeof(vec_h_t);
    for(int i = 0; i < vec_h->size; )
    {
        int len = min(vec_h->size - i, (PAGE_SIZE-(pos%PAGE_SIZE))/(int)sizeof(int));
        if (append_only && i < append_only_offset) len = min(len, append_only_offset - i);
        if (!append_only || i >= append_only_offset) memcpy(file->ReadPage(pos/PAGE_SIZE)->data()+(pos%PAGE_SIZE), data->data()+i, len*sizeof(int));
        pos += len*sizeof(int);
        i += len;
    }

    return npos;
}

void VectorFile::Flush()
{
    file->Flush();
}