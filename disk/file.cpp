#include "file.h"
#include <cassert>
#include <cstdio>
#include <iostream>

using namespace std;

File::File(string filename) : filename(filename)
{
    next_page = total_page = 0;
    if (exists(filename))
    {
        total_page = filesize(filename) / PAGE_SIZE;
    } else {
        mkfile(filename);
    }
}

File::~File()
{
    for(auto x : dirty_marks)
    {
        SavePage(x, data_pool[x]);
    }
}

void File::ResetNextPage(int page)
{
    next_page = page;
}

data_t File::NextPage(bool read_only)
{
    return ReadPage(next_page);
}

data_t File::ReadPage(int page, bool read_only)
{
    data_t data;
    if (data_pool.find(page) != data_pool.end())
    {
        data = data_pool[page];
    } else if (page < total_page) {
        data = LoadPage(page);
    } else {
        data = alloc_data(PAGE_SIZE);
    }
    data_pool[page] = data;
    next_page = page + 1;
    total_page = max(total_page, next_page);
    if (!read_only) dirty_marks.insert(page);
    return data;
}

int File::CurrentPage()
{
    return next_page - 1;
}

bool File::End()
{
    return next_page >= total_page;
}

data_t File::LoadPage(int page)
{
    data_t data = alloc_data(PAGE_SIZE);

    FILE* fd = fopen(filename.c_str(), "rb");
    assert(fd);
    fseek(fd, page*PAGE_SIZE, SEEK_SET);
    fread(data->data(), 1, PAGE_SIZE, fd);
    fclose(fd);

    return data;
}

void File::SavePage(int page, data_t data)
{
    FILE* fd = fopen(filename.c_str(), "r+b");
    assert(fd);
    fseek(fd, page*PAGE_SIZE, SEEK_SET);
    fwrite(data->data(), 1, PAGE_SIZE, fd);
    fclose(fd);
}
