// 基础磁盘操作
#ifndef DISK_COMMON_H
#define DISK_COMMON_H

#include <memory>
#include <string>
#include <vector>

using namespace std;

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;

typedef shared_ptr<vector<uint8>> data_t;

data_t alloc_data(int size);
void mkdirp(const string& path);
void mkfile(const string& filepath);
void rmdir(const string& path);
void rmfile(const string& filepath);
bool exists(const string& path);
int filesize(const string& filepath);

string path_join(const string& a, const string& b);
string get_cwd();

void debug(data_t data);

#endif // DISK_COMMON_H