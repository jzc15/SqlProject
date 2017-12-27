// 数据管理，使用槽管理
#ifndef DATA_MANAGER_SLOTS_FILE_H
#define DATA_MANAGER_SLOTS_FILE_H

#include <memory>
#include <string>
#include <disk/common.h>
#include <disk/file.h>

using namespace std;

// RID = page_id + slot_id, 均从0开始
#define RID(page_id, slot_id) int( (int(page_id) << 13) | int(slot_id))
#define RID_PAGE_ID(rid) int(int(rid) >> 13)
#define RID_SLOT_ID(rid) int(int(rid) & ((1<<13)-1))

class SlotsFile
{
public:
    typedef shared_ptr<SlotsFile> ptr;

    SlotsFile(const string& filename);

    // 插入，返回rid
    int Insert(data_t data);

    // 删除，不会移动其他数据
    void Delete(int rid);

    // 获取，如果没有则返回nullptr
    data_t Fetch(int rid);

    // 开始的数据，如果没有则返回nullptr
    data_t Begin();
    // 当前rid
    int CurrentRID();
    // 下一个数据，如果没有则返回nullptr
    data_t Next();

private:
    struct header_t
    {
        int valid;
        int last_page_id;
    };

private:
    string filename;
    File::ptr file;
    header_t* header;
    int current_rid;
};

#endif // DATA_MANAGER_SLOTS_FILE_H