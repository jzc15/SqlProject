#include "hashtable.h"
#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

HashTable::HashTable(const string& filename, int key_bytes)
    : filename(filename)
{
    this->file = make_shared<File>(filename);
    this->header = initTable(file, key_bytes);
}
HashTable::~HashTable()
{
}

void HashTable::RemoveIndex(const string& filename)
{
    if (::exists(filename)) rmfile(filename);
}

void HashTable::Insert(data_t key, int value)
{
    header->size ++;
    ins(file, key->data(), value, header->key_bytes, header->P, header->M);
    if (header->size > header->M * 0.75)
    {
        const string tmp_filename = filename + "_tmp";
        if (::exists(tmp_filename)) rmfile(tmp_filename);

        int key_bytes = header->key_bytes;
        extendCopy(file, tmp_filename, header->key_bytes, header->P, header->M);
        file->Flush();
        file = nullptr;
        rmfile(filename);
        cpfile(tmp_filename, filename);
        rmfile(tmp_filename);
        file = make_shared<File>(filename);
        header = initTable(file, key_bytes);

        // cout << "Extend to M = " << header->M << endl;
    }
}
void HashTable::Delete(data_t key)
{
    header->size --;
    del(file, key->data(), header->key_bytes, header->P, header->M);
}
bool HashTable::Exists(data_t key)
{
    return exists(file, key->data(), header->key_bytes, header->P, header->M);
}
int HashTable::Fetch(data_t key)
{
    return get(file, key->data(), header->key_bytes, header->P, header->M);
}

int HashTable::TotalRecords()
{
    return header->size;
}

HashTable::header_t* HashTable::initTable(File::ptr file, int key_bytes)
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
    }
    return header;
}

int HashTable::nextPrime(int current)
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

int HashTable::hashValue(uint8* key, int key_bytes, int P, int M)
{
    int ret = 0;
    for(int i = 0; i < key_bytes; i ++)
    {
        ret = (ret * P + (int)key[i]) % M;
    }
    return ret;
}

void HashTable::ins(File::ptr file, uint8* key, int value, int key_bytes, int P, int M)
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
            if (node_h->type == NODE_TYPE_NONE || node_h->type == NODE_TYPE_UNUSE)
            {
                node_h->type = NODE_TYPE_INUSE;
                node_h->value = value;
                memcpy((uint8*)node_h+sizeof(node_h_t), key, key_bytes);
                file->MarkDirty(page_id);
                return;
            }

            factor = -factor;
        }
    }
    assert(false);
}
void HashTable::del(File::ptr file, uint8* key, int key_bytes, int P, int M)
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
                node_h->type = NODE_TYPE_UNUSE;
                file->MarkDirty(page_id);
                return;
            }

            factor = -factor;
        }
    }
    assert(false);
}
bool HashTable::exists(File::ptr file, uint8* key, int key_bytes, int P, int M)
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
                return true;
            }
            if (node_h->type == NODE_TYPE_NONE) return false;

            factor = -factor;
        }
    }
    assert(false);
    return false;
}
int HashTable::get(File::ptr file, uint8* key, int key_bytes, int P, int M)
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
                return node_h->value;
            }

            factor = -factor;
        }
    }
    assert(false);
    return -1;
}
void HashTable::extendCopy(File::ptr file, const string& newfilename, int key_bytes, int P, int M)
{
    const int entry_size = sizeof(node_h_t)+key_bytes;
    const int page_entries = PAGE_SIZE/entry_size;

    File::ptr newfile = make_shared<File>(newfilename);
    header_t* nheader = initTable(newfile, key_bytes);
    nheader->M = nextPrime(M*2);
    for(int pos = 0; pos < M; pos ++)
    {
        int page_id = pos / page_entries + 1;
        int inpage_id = pos % page_entries;

        node_h_t* node_h = (node_h_t*)(file->ReadPage(page_id, true)->data() + inpage_id*entry_size);
        if (node_h->type == NODE_TYPE_INUSE)
        {
            nheader->size ++;
            ins(newfile, (uint8*)node_h+sizeof(node_h_t), node_h->value, key_bytes, P, nheader->M);
        }
    }
    newfile->Flush();
}