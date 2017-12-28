#ifndef DISK_FILECACHE_H
#define DISK_FILECACHE_H

#include <map>
#include <set>
#include <string>
#include <cstdio>
#include <queue>
#include "common.h"

using namespace std;

#define PAGE_SIZE 8192 // 8KB
#define IN_MEM_PAGES 0 // 最大内存保留页数

// 缓存
class FileCache
{
public:
    FileCache();
    ~FileCache();

    int Register(const string& filename); // 注册，在取消注册之前不会删除其相关联的内存
    void Unregister(int file_id); // 取消注册

    data_t LoadPage(int file_id, int page_id); // 读
    void MarkDirty(int file_id, int page_id); // 设上脏标志
    int TotalPage(int file_id); // 总页数

    void Flush(int file_id);

private:
    struct Node
    {
        int timestamp;
        int mid;

        bool operator<(const Node& B) const
        {
            return timestamp > B.timestamp;
        }
    };

private:
    vector<string> filenames;
    vector<int> registered;
    vector<set<int>> registered_mids; // 当前注册文件的mid列表
    vector<int> disk_total_pages;
    vector<int> mem_total_pages;
    map<string, int> fids;

    map<int, data_t> caches; // 当前的数据, mid->data
    set<int> dirty_marks; // 脏标记, mid
    map<int, long long> timestamps; // 时间戳, mid->timestamp
    priority_queue<Node> heap;
    long long current_time;

    void Update();
    static data_t LoadPage(const string& filename, int page);
    static void SavePage(const string& filename, int page, data_t data);
};

#endif // DISK_FILECACHE_H