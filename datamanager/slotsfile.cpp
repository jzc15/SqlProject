#include "slotsfile.h"
#include <cstring>
#include <cassert>
#include <iostream>

using namespace std;

SlotsFile::SlotsFile(const string& filename)
    : filename(filename)
{
    file = make_shared<File>(filename);
    current_rid = 0;
}

// 插入，返回rid
int SlotsFile::Insert(data_t data)
{
    file->ResetNextPage();
    while(true)
    {
        data_t page = file->NextPage(true);
        int free_offset = *(int*)(page->data()+PAGE_SIZE-4);
        int n_slots = *(int*)(page->data()+PAGE_SIZE-8);
        int free_space = PAGE_SIZE - 8 - n_slots*8 - free_offset;
        if (free_space >= (int)data->size() + 8) break;
    }

    data_t page = file->ReadPage(file->CurrentPage(), false);
    int free_offset = *(int*)(page->data()+PAGE_SIZE-4);
    int n_slots = *(int*)(page->data()+PAGE_SIZE-8);
    
    memcpy(page->data() + free_offset, data->data(), data->size());
    *(int*)(page->data()+PAGE_SIZE-8) = n_slots+1;
    *(int*)(page->data()+PAGE_SIZE-8-n_slots*8-8) = free_offset;
    *(int*)(page->data()+PAGE_SIZE-8-n_slots*8-4) = data->size();
    free_offset += data->size();
    *(int*)(page->data()+PAGE_SIZE-4) = free_offset;

    return RID(file->CurrentPage(), n_slots);
}

// 删除，不会移动其他数据
void SlotsFile::Delete(int rid)
{
    int page_id = RID_PAGE_ID(rid);
    int slot_id = RID_SLOT_ID(rid);
    data_t page = file->ReadPage(page_id);
    int n_slots = *(int*)(page->data()+PAGE_SIZE-8);
    assert(slot_id < n_slots);

    *(int*)(page->data()+PAGE_SIZE-8-slot_id*8-8) = -1;
}

// 获取，如果没有则返回nullptr
data_t SlotsFile::Fetch(int rid)
{
    int page_id = RID_PAGE_ID(rid);
    int slot_id = RID_SLOT_ID(rid);
    data_t page = file->ReadPage(page_id, true);
    int n_slots = *(int*)(page->data()+PAGE_SIZE-8);

    if(slot_id >= n_slots) return nullptr;

    int offset = *(int*)(page->data()+PAGE_SIZE-8-slot_id*8-8);
    int size = *(int*)(page->data()+PAGE_SIZE-8-slot_id*8-4);
    if (offset < 0) return nullptr;

    data_t data = alloc_data(size);
    memcpy(data->data(), page->data()+offset, size);
    return data;
}

// 开始的数据，如果没有则返回nullptr
data_t SlotsFile::Begin()
{
    current_rid = RID(0, 0);
    data_t first = Fetch(current_rid);
    if (first != nullptr) return first;
    return Next();
}

int SlotsFile::CurrentRID()
{
    return current_rid;
}

// 下一个数据，如果没有则返回nullptr
data_t SlotsFile::Next()
{
    int page_id = RID_PAGE_ID(current_rid);
    int slot_id = RID_SLOT_ID(current_rid) + 1;
    while(true)
    {
        data_t page = file->ReadPage(page_id, true);
        int n_slots = *(int*)(page->data()+PAGE_SIZE-8);
        while(slot_id < n_slots)
        {
            int offset = *(int*)(page->data()+PAGE_SIZE-8-slot_id*8-8);
            // int size = *(int*)(page->data()+PAGE_SIZE-8-slot_id*8-4);
            if (offset >= 0) break;
            slot_id ++;
        }
        if (slot_id < n_slots)
        {
            current_rid = RID(page_id, slot_id);
            return Fetch(current_rid);
        }
        if (file->End()) return nullptr;
        page_id ++;
        slot_id = 0;
    }
}