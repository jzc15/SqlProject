#include "common.h"
#include <unistd.h> 
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

data_t alloc_data(int size)
{
    data_t data = make_shared<vector<uint8>>();
    data->resize(size);
    memset(data->data(), 0, size);
    return data;
}

void mkdirp(const string& path)
{
    char buf[1024];
    sprintf(buf, "mkdir -p '%s'", path.c_str());
    assert(system(buf) == 0);
}

void mkfile(const string& filepath)
{
    char buf[1024];
    sprintf(buf, "touch '%s'", filepath.c_str());
    assert(system(buf) == 0);
}

void rmdir(const string& path)
{
    char buf[1024];
    sprintf(buf, "rm -rf '%s'", path.c_str());
    assert(system(buf) == 0);
}

void rmfile(const string& filepath)
{
    char buf[1024];
    sprintf(buf, "rm '%s'", filepath.c_str());
    assert(system(buf) == 0);
}

bool exists(const string& path)
{
    return access(path.c_str(), F_OK) == 0;
}

int filesize(const string& filepath)
{
    FILE* fd = fopen(filepath.c_str(), "rb");
    fseek(fd, 0L, SEEK_END);
    int sz = ftell(fd);
    fclose(fd);

    return sz;
}

string path_join(const string& a, const string& b)
{
    if (a.length() == 0) return b;
    if (b.length() == 0) return a;
    if (a[a.length()-1] == '/' || b[0] == '/') return a + b;
    return a + "/" + b;
}

string get_cwd()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    return cwd;
}

void debug(data_t data)
{
    static char* HEX = "0123456789ABCDEF";
    for(int i = 0; i < (int)data->size(); i ++)
    {
        cout << HEX[data->data()[i]>>4] << HEX[data->data()[i]&15] << " ";
    }
    cout << endl;
}