#ifndef ENGINE_OPTIONS_H
#define ENGINE_OPTIONS_H

// 选项

#include <ostream>
using namespace std;

extern bool debug_on; // 是否打开输出调试
extern ostream *out; // 标准输出流
extern ostream *err; // 错误输出流

#endif // ENGINE_OPTIONS_H