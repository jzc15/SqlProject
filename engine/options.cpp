#include "options.h"
#include <iostream>
using namespace std;

bool debug_on = true; // 是否打开输出调试
ostream *out = &cout; // 标准输出流
ostream *err = &cerr; // 错误输出流