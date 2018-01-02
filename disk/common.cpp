#include "common.h"
#include <unistd.h> 
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

bool equals(data_t a, data_t b)
{
    if (a->size() != b->size()) return false;
    for(int i = 0; i < (int)a->size(); i ++)
    {
        if (a->data()[i] != b->data()[i]) return false;
    }
    return true;
}

data_t alloc_data(int size)
{
    data_t data = make_shared<vector<uint8>>();
    data->resize(size);
    memset(data->data(), 0, size);
    return data;
}

data_t clone(data_t data)
{
    if (data == nullptr) return nullptr;
    data_t d = alloc_data(data->size());
    memcpy(d->data(), data->data(), data->size());
    return d;
}

data_t int_data(int value)
{
    data_t data = alloc_data(sizeof(int));
    *(int*)(data->data()) = value;
    return data;
}

data_t float_data(float value)
{
    data_t data = alloc_data(sizeof(float));
    *(float*)(data->data()) = value;
    return data;
}

data_t string_data(string str)
{
    data_t data = alloc_data(str.length());
    memcpy(data->data(), str.c_str(), str.length());
    return data;
}

data_t time_data(time_t value)
{
    data_t data = alloc_data(sizeof(time_t));
    *(time_t*)(data->data()) = value;
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

void cpfile(const string& src, const string& dst)
{
    char buf[1024];
    sprintf(buf, "cp '%s' '%s'", src.c_str(), dst.c_str());
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

vector<string> listdir(const string& path)
{
    DIR *dp;
    struct dirent *ep;
    vector<string> ans;

    dp = opendir(path.c_str());
    if (dp != NULL)
    {
        while (ep = readdir(dp))
        {
            if (ep->d_name != string(".") && ep->d_name != string(".."))
                ans.push_back(ep->d_name);
        }
        closedir(dp);
    }

    return ans;
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
    debug(data->data(), data->size());
}

void debug(uint8* data, int len)
{
    static char* HEX = "0123456789ABCDEF";
    for(int i = 0; i < len; i ++)
    {
        cout << HEX[data[i]>>4] << HEX[data[i]&15] << " ";
    }
    cout << endl;
}
