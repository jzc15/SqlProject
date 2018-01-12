#ifndef INDICES_HASHTABLE_H
#define INDICES_HASHTABLE_H

#include <disk/common.h>
#include <disk/file.h>
#include <memory>

using namespace std;

// hash表，key唯一
class HashTable
{
public:
    typedef shared_ptr<HashTable> ptr;
    HashTable(const string& filename, int key_bytes);
    ~HashTable();

    static void RemoveIndex(const string& filename);

    void Insert(data_t key, int value); // 插入
    void Delete(data_t key); // 删除
    bool Exists(data_t key); // 是否存在
    int Fetch(data_t key); // 获取

    int TotalRecords(); // 记录总数

private:
    struct header_t // 页首
    {
        int valid;
        int key_bytes;
        int P, M;
        int size;
    };
    struct node_h_t
    {
        int value;
        char type;
    };

private:
    const string filename;
    File::ptr file;
    header_t* header;

private:
    static header_t* initTable(File::ptr file, int key_bytes);
    static int nextPrime(int current);
    static int hashValue(uint8* key, int key_bytes, int P, int M);
    static void ins(File::ptr file, uint8* key, int value, int key_bytes, int P, int M);
    static void del(File::ptr file, uint8* key, int key_bytes, int P, int M);
    static bool exists(File::ptr file, uint8* key, int key_bytes, int P, int M);
    static int get(File::ptr file, uint8* key, int key_bytes, int P, int M);
    static void extendCopy(File::ptr file, const string& newfilename, int key_bytes, int P, int M);
private:
    static const int NODE_TYPE_NONE = 0;
    static const int NODE_TYPE_INUSE = 1;
    static const int NODE_TYPE_UNUSE = 2;
};

#endif // INDICES_HASHTABLE_H