// 8K页式文件系统
#ifndef DISK_FILE_H
#define DISK_FILE_H

#include "common.h"
#include <memory>
#include <string>
#include <map>
#include <set>

using namespace std;

#define PAGE_SIZE 8192 // 8KB

// 页式文件
class File
{
public:
    typedef shared_ptr<File> ptr;

    File(string filename);
    ~File();

    // 
    void ResetNextPage(int page = 0);

    // 读取一页，若该页不存在则返回全0的数据
    data_t ReadPage(int page, bool read_only = false);

    // 读取下一页，若该页不存在则返回全0的数据
    data_t NextPage(bool read_only = false);

    // 新页，在最后
    data_t NewPage(bool read_only = false);

    // 返回当前页
    int CurrentPage();

    // 是否结束
    bool End();

    void MarkDirty(int page);

    void Close();

private:
    string filename;
    int next_page;
    int total_page;
    map<int, data_t> data_pool;
    set<int> dirty_marks;

    data_t LoadPage(int page);
    void SavePage(int page, data_t data);
};

#endif // DISK_FILE_H