#ifndef DATA_MANAGER_VECTOR_FILE_H
#define DATA_MANAGER_VECTOR_FILE_H

#include <disk/file.h>
#include <memory>
using namespace std;

typedef shared_ptr<vector<int>> vector_t;

// 储存int
class VectorFile
{
public:
    typedef shared_ptr<VectorFile> ptr;
    VectorFile(const string& filename);
    ~VectorFile();

    int NewVector(int value);
    vector_t Fetch(int pos);
    int Save(int opos, vector_t data);

    void Close();

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