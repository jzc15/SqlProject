#include "filecache.h"
#include <cassert>
#include <iostream>

using namespace std;

#define MIX(file_id, page_id) ((int(page_id) << 10) | int(file_id))
#define MPAGE(mid) (int(mid)>>10)
#define MFILE(mid) (int(mid)&((1<<10)-1))

static bool cache_debug = false;

FileCache::FileCache()
{
    current_time = 0;
}
FileCache::~FileCache()
{
    for(auto mid : dirty_marks)
    {
        int fid = MFILE(mid);
        int page = MPAGE(mid);
        SavePage(filenames[fid], page, caches[mid]);
    }
}

int FileCache::Register(const string& filename)
{
    int fid = -1;

    if (fids.find(filename) != fids.end())
    {
        fid = fids.at(filename);
    } else {
        fid = fids[filename] = filenames.size();
        filenames.resize(fid+1);
        registered.resize(fid+1);
        registered_mids.resize(fid+1);
        disk_total_pages.resize(fid+1);
        mem_total_pages.resize(fid+1);

        filenames[fid] = filename;
        disk_total_pages[fid] = 0;
        mem_total_pages[fid] = 0;
        if (exists(filename))
        {
            mem_total_pages[fid] = disk_total_pages[fid] = filesize(filename) / PAGE_SIZE;
        } else {
            mkfile(filename);
        }
    }

    registered[fid] ++;
    if (cache_debug) cout << "Register `" << filename << "` : " << fid << endl;

    return fid;
}
void FileCache::Unregister(int fid)
{
    if (cache_debug) cout << "Unregister `" << filenames[fid] << "`" << endl;

    registered[fid] --;
    if (registered[fid] == 0)
    {
        for(auto mid : registered_mids[fid])
        {
            Node node;
            node.mid = mid;
            node.timestamp = timestamps[mid];
            heap.push(node);
        }
        registered_mids[fid].clear();
        Update();
    }
}

void FileCache::Flush(int fid)
{
    for(auto mid : dirty_marks)
    {
        SavePage(filenames[MFILE(mid)], MPAGE(mid), caches[mid]);
    }
    dirty_marks.clear();
    caches.clear();
    for(auto& x : registered_mids)
    {
        x.clear();
    }
    fids.erase(fids.find(filenames[fid]));
}

data_t FileCache::LoadPage(int file_id, int page_id)
{
    if (cache_debug) cout << "LoadPage `" << filenames[file_id] << "`(" << file_id << ") : " << page_id << endl;

    const int mid = MIX(file_id, page_id);
    data_t data = nullptr;
    if (caches.find(mid) != caches.end())
    {
        if (cache_debug) cout << "[1]" << endl;
        data = caches.at(mid);
    } else {
        if (page_id < disk_total_pages[file_id])
        {
            if (cache_debug) cout << "[2]" << endl;
            data = LoadPage(filenames[file_id], page_id);
        } else {
            if (cache_debug) cout << "[3]" << endl;
            data = alloc_data(PAGE_SIZE);
        }
    }
    caches[mid] = data;
    timestamps[mid] = current_time ++;
    registered_mids[file_id].insert(mid);
    mem_total_pages[file_id] = max(mem_total_pages[file_id], page_id+1);
    Update();
    return data;
}
void FileCache::MarkDirty(int file_id, int page_id)
{
    dirty_marks.insert(MIX(file_id, page_id));
}
int FileCache::TotalPage(int file_id)
{
    return mem_total_pages[file_id];
}

void FileCache::Update()
{
    while(caches.size() > IN_MEM_PAGES)
    {
        while(!heap.empty() && timestamps[heap.top().mid] != heap.top().timestamp) heap.pop();
        if (heap.empty()) break;
        Node node = heap.top(); heap.pop();
        int mid = node.mid;
        int fid = MFILE(mid);
        int page = MPAGE(mid);
        if (registered[fid] && registered_mids[fid].find(mid) != registered_mids[fid].end()) continue;
        data_t data = caches.at(mid);
        caches.erase(caches.find(mid));
        if (cache_debug) cout << "Delete `" << filenames[fid] << "` : " << page << " from caches" << endl;
        if (dirty_marks.find(mid) != dirty_marks.end())
        {
            dirty_marks.erase(dirty_marks.find(mid));
            SavePage(filenames[fid], page, data);
            if (cache_debug) cout << "Write back `" << filenames[fid] << "` : " << page << endl;
        }
    }
}

data_t FileCache::LoadPage(const string& filename, int page)
{
    data_t data = alloc_data(PAGE_SIZE);

    FILE* fd = fopen(filename.c_str(), "rb");
    assert(fd);
    fseek(fd, page*PAGE_SIZE, SEEK_SET);
    fread(data->data(), 1, PAGE_SIZE, fd);
    fclose(fd);

    return data;
}

void FileCache::SavePage(const string& filename, int page, data_t data)
{
    FILE* fd = fopen(filename.c_str(), "r+b");
    assert(fd);
    fseek(fd, page*PAGE_SIZE, SEEK_SET);
    fwrite(data->data(), 1, PAGE_SIZE, fd);
    fclose(fd);
}