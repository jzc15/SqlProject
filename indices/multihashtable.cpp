#include "multihashtable.h"
#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

MultiHashTable::MultiHashTable(const string& filename, int key_bytes)
    : filename(filename)
{
    this->file = make_shared<File>(filename);
    this->data_file = make_shared<VectorFile>(filename + ".data");
    this->header = initTable(file, key_bytes);
}
MultiHashTable::~MultiHashTable()
{
}

void MultiHashTable::RemoveIndex(const string& filename)
{
    if (::exists(filename)) rmfile(filename);
    if (::exists(filename + ".data")) rmfile(filename + ".data");
}

void MultiHashTable::Insert(data_t key, int value)
{
    header->total_records ++;
    ins(file, data_file, key->data(), value, header->key_bytes, header->P, header->M);
    if (header->size > header->M * 0.75)
    {
        const string tmp_filename = filename + "_tmp";
        if (::exists(tmp_filename)) rmfile(tmp_filename);
        if (::exists(tmp_filename + ".data")) rmfile(tmp_filename + ".data");

        int key_bytes = header->key_bytes;
        extendCopy(file, data_file, tmp_filename, header->key_bytes, header->P, header->M);
        file->Flush();
        file = nullptr;
        data_file->Flush();
        data_file = nullptr;
        rmfile(filename);
        cpfile(tmp_filename, filename);
        rmfile(tmp_filename);
        rmfile(filename + ".data");
        cpfile(tmp_filename + ".data", filename + ".data");
        rmfile(tmp_filename + ".data");
        file = make_shared<File>(filename);
        data_file = make_shared<VectorFile>(filename + ".data");
        header = initTable(file, key_bytes);

        cout << "Extend to M = " << header->M << endl;
    }
}
void MultiHashTable::Delete(data_t key, int value)
{
    header->total_records --;
    del(file, data_file, key->data(), value, header->key_bytes, header->P, header->M);
}
int MultiHashTable::Count(data_t key)
{
    return count(file, data_file, key->data(), header->key_bytes, header->P, header->M);
}
vector_t MultiHashTable::Fetch(data_t key)
{
    return get(file, data_file, key->data(), header->key_bytes, header->P, header->M);
}

int MultiHashTable::TotalRecords()
{
    return header->total_records;
}

MultiHashTable::header_t* MultiHashTable::initTable(File::ptr file, int key_bytes)
{
    header_t* header = (header_t*)file->ReadPage(0)->data();
    if (header->valid)
    {
        assert(key_bytes == header->key_bytes);
    } else {
        header->valid = 1;
        header->key_bytes = key_bytes;
        header->P = 241;
        header->M = nextPrime(1024);
        // header->M = nextPrime(1);
        header->size = 0;
        header->total_records = 0;
    }
    return header;
}

int MultiHashTable::nextPrime(int current)
{
    while(1)
    {
        current ++;
        if (current % 4 != 3) continue;

        bool flag = false;
        for(int i = 2; i * i <= current && !flag; i ++)
            if (current % i == 0) flag = true;
        if (!flag) return current;
    }
}

int MultiHashTable::hashValue(uint8* key, int key_bytes, int P, int M)
{
    int ret = 0;
    for(int i = 0; i < key_bytes; i ++)
    {
        ret = (ret * P + (int)key[i]) % M;
    }
    return ret;
}

void MultiHashTable::ins(File::ptr file, VectorFile::ptr data_file, uint8* key, int value, int key_bytes, int P, int M)
{
    header_t* header = (header_t*)file->ReadPage(0)->data();
    const int entry_size = sizeof(node_h_t)+key_bytes;
    const int page_entries = PAGE_SIZE/entry_size;

    int hash_key = hashValue(key, key_bytes, P, M);
    for(int i = 0; i <= M / 2; i ++)
    {
        int factor = i*i;
        for(int j = 0; j < 2; j ++)
        {
            int pos = ((hash_key + factor) % M + M) % M;
            int page_id = pos / page_entries + 1;
            int inpage_id = pos % page_entries;

            node_h_t* node_h = (node_h_t*)(file->ReadPage(page_id, true)->data() + inpage_id*entry_size);
            if (node_h->type == NODE_TYPE_NONE) {
                header->size ++; // 被占用的位置++
                node_h->type = NODE_TYPE_INUSE;
                node_h->data_pos = data_file->NewVector(value);
                memcpy((uint8*)node_h + sizeof(node_h_t), key, key_bytes);
                file->MarkDirty(page_id);
                return;
            } else if (node_h->type == NODE_TYPE_INUSE && 
                memcmp((uint8*)node_h + sizeof(node_h_t), key, key_bytes) == 0) {
                vector_t data = data_file->Fetch(node_h->data_pos);
                data->push_back(value);
                node_h->data_pos = data_file->Save(node_h->data_pos, data, true);
                file->MarkDirty(page_id);
                return;
            }

            factor = -factor;
        }
    }
    assert(false);
}
void MultiHashTable::del(File::ptr file, VectorFile::ptr data_file, uint8* key, int value, int key_bytes, int P, int M)
{
    const int entry_size = sizeof(node_h_t)+key_bytes;
    const int page_entries = PAGE_SIZE/entry_size;

    int hash_key = hashValue(key, key_bytes, P, M);
    for(int i = 0; i <= M / 2; i ++)
    {
        int factor = i*i;
        for(int j = 0; j < 2; j ++)
        {
            int pos = ((hash_key + factor) % M + M) % M;
            int page_id = pos / page_entries + 1;
            int inpage_id = pos % page_entries;

            node_h_t* node_h = (node_h_t*)(file->ReadPage(page_id, true)->data() + inpage_id*entry_size);
            if (node_h->type == NODE_TYPE_INUSE &&
                memcmp((uint8*)node_h+sizeof(node_h_t), key, key_bytes) == 0)
            {
                vector_t data = data_file->Fetch(node_h->data_pos);
                for(int i = 0; i+1 < (int)data->size(); i ++)
                    if (data->at(i) == value)
                        swap(data->data()[i], data->data()[i+1]);
                assert(data->at(data->size()-1) == value);
                data->pop_back();
                node_h->data_pos = data_file->Save(node_h->data_pos, data);
                file->MarkDirty(page_id);
                return;
            }

            factor = -factor;
        }
    }
    assert(false);
}
int MultiHashTable::count(File::ptr file, VectorFile::ptr data_file, uint8* key, int key_bytes, int P, int M)
{
    const int entry_size = sizeof(node_h_t)+key_bytes;
    const int page_entries = PAGE_SIZE/entry_size;

    int hash_key = hashValue(key, key_bytes, P, M);
    for(int i = 0; i <= M / 2; i ++)
    {
        int factor = i*i;
        for(int j = 0; j < 2; j ++)
        {
            int pos = ((hash_key + factor) % M + M) % M;
            int page_id = pos / page_entries + 1;
            int inpage_id = pos % page_entries;

            node_h_t* node_h = (node_h_t*)(file->ReadPage(page_id, true)->data() + inpage_id*entry_size);
            if (node_h->type == NODE_TYPE_INUSE &&
                memcmp((uint8*)node_h+sizeof(node_h_t), key, key_bytes) == 0)
            {
                return data_file->Fetch(node_h->data_pos)->size();
            }
            if (node_h->type == NODE_TYPE_NONE) return 0;

            factor = -factor;
        }
    }
    assert(false);
    return false;
}
vector_t MultiHashTable::get(File::ptr file, VectorFile::ptr data_file, uint8* key, int key_bytes, int P, int M)
{
    const int entry_size = sizeof(node_h_t)+key_bytes;
    const int page_entries = PAGE_SIZE/entry_size;

    int hash_key = hashValue(key, key_bytes, P, M);
    for(int i = 0; i <= M / 2; i ++)
    {
        int factor = i*i;
        for(int j = 0; j < 2; j ++)
        {
            int pos = ((hash_key + factor) % M + M) % M;
            int page_id = pos / page_entries + 1;
            int inpage_id = pos % page_entries;

            node_h_t* node_h = (node_h_t*)(file->ReadPage(page_id, true)->data() + inpage_id*entry_size);
            if (node_h->type == NODE_TYPE_INUSE &&
                memcmp((uint8*)node_h+sizeof(node_h_t), key, key_bytes) == 0)
            {
                return data_file->Fetch(node_h->data_pos);
            }

            factor = -factor;
        }
    }
    assert(false);
    // return -1;
}
void MultiHashTable::extendCopy(File::ptr file, VectorFile::ptr data_file, const string& newfilename, int key_bytes, int P, int M)
{
    const int entry_size = sizeof(node_h_t)+key_bytes;
    const int page_entries = PAGE_SIZE/entry_size;

    File::ptr newfile = make_shared<File>(newfilename);
    VectorFile::ptr newdatafile = make_shared<VectorFile>(newfilename + ".data");

    header_t* nheader = initTable(newfile, key_bytes);
    nheader->M = nextPrime(M*2);
    for(int pos = 0; pos < M; pos ++)
    {
        int page_id = pos / page_entries + 1;
        int inpage_id = pos % page_entries;

        node_h_t* node_h = (node_h_t*)(file->ReadPage(page_id, true)->data() + inpage_id*entry_size);
        if (node_h->type == NODE_TYPE_INUSE)
        {
            vector_t data = data_file->Fetch(node_h->data_pos);
            for(int i = 0; i < (int)data->size(); i ++)
                ins(newfile, newdatafile, (uint8*)node_h+sizeof(node_h_t),data->at(i), key_bytes, P, nheader->M);
        }
    }
    newfile->Flush();
    newdatafile->Flush();
}
