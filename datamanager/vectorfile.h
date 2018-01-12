#ifndef DATA_MANAGER_VECTOR_FILE_H
#define DATA_MANAGER_VECTOR_FILE_H

#include <disk/file.h>
#include <memory>
#include <map>
using namespace std;

typedef shared_ptr<vector<int>> vector_t;

// 储存int数组
class VectorFile
{
public:
    typedef shared_ptr<VectorFile> ptr;
    VectorFile(const string& filename);
    ~VectorFile();

    int NewVector(int value); // 新数组，返回位置
    vector_t Fetch(int pos); // 获取对应位置的数组
    int Save(int opos, vector_t data, bool append_only = false); // 储存，append_only=是否只有增加

    void Flush();

private:
    struct header_t
    {
        int valid;
        int empty_pos;
    };
    struct vec_h_t
    {
        int size, capacity;
    };

private:
    string filename;
    File::ptr file;
    header_t* header;
};

#endif // DATA_MANAGER_VECTOR_FILE_H