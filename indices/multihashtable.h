#ifndef INDICES_MULTI_HASHTABLE_H
#define INDICES_MULTI_HASHTABLE_H

#include <disk/common.h>
#include <datamanager/vectorfile.h>
#include <memory>

using namespace std;

// hash表，可以有多个相同的key
class MultiHashTable
{
public:
    typedef shared_ptr<MultiHashTable> ptr;
    MultiHashTable(const string& filename, int key_bytes);
    ~MultiHashTable();

    static void RemoveIndex(const string& filename);

    void Insert(data_t key, int value);
    void Delete(data_t key, int value);
    int Count(data_t key);
    vector_t Fetch(data_t key);

    int TotalRecords();

private:
    struct header_t // 页首
    {
        int valid;
        int key_bytes;
        int P, M;
        int size;
        int total_records;
    };
    struct node_h_t
    {
        int data_pos;
        char type;
    };

private:
    const string filename;
    File::ptr file;
    VectorFile::ptr data_file;
    header_t* header;

private:
    static header_t* initTable(File::ptr file, int key_bytes);
    static int nextPrime(int current);
    static int hashValue(uint8* key, int key_bytes, int P, int M);
    static void ins(File::ptr file, VectorFile::ptr data_file, uint8* key, int value, int key_bytes, int P, int M);
    static void del(File::ptr file, VectorFile::ptr data_file, uint8* key, int value, int key_bytes, int P, int M);
    static int count(File::ptr file, VectorFile::ptr data_file, uint8* key, int key_bytes, int P, int M);
    static vector_t get(File::ptr file, VectorFile::ptr data_file, uint8* key, int key_bytes, int P, int M);
    static void extendCopy(File::ptr file, VectorFile::ptr data_file, const string& newfilename, int key_bytes, int P, int M);
private:
    static const int NODE_TYPE_NONE = 0;
    static const int NODE_TYPE_INUSE = 1;
    // static const int NODE_TYPE_UNUSE = 2;
};

#endif // INDICES_MULTI_HASHTABLE_H