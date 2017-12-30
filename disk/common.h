// 基础磁盘操作
#ifndef DISK_COMMON_H
#define DISK_COMMON_H

#include <memory>
#include <string>
#include <vector>
#include <cstring>

using namespace std;

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;

typedef shared_ptr<vector<uint8>> data_t;

bool equals(data_t a, data_t b);
data_t alloc_data(int size);
data_t clone(data_t data);
data_t int_data(int value);
data_t float_data(float value);
data_t string_data(string str);
data_t time_data(time_t value);

void mkdirp(const string& path);
void mkfile(const string& filepath);
void rmdir(const string& path);
void rmfile(const string& filepath);
void cpfile(const string& src, const string& dst);
bool exists(const string& path);
int filesize(const string& filepath);
vector<string> listdir(const string& path);

string path_join(const string& a, const string& b);
string get_cwd();

template<typename T>
void append(data_t data, T value)
{
    uint8 buf[sizeof(T)];
    memcpy(buf, &value, sizeof(T));
    // *static_cast<T*>(buf) = value;
    for(int i = 0; i < (int)sizeof(T); i ++)
        data->push_back(buf[i]);
}
void debug(data_t data);
void debug(uint8* data, int len);

#endif // DISK_COMMON_H