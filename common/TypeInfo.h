#ifndef _TYPE_INFO_H
#define _TYPE_INFO_H

#include <string>
using namespace std;

// 类型信息，类型均用小写字符串表示
/**
 * 目前支持的类型：
 * int 整数,定长
 * char 字符,定长
 * char(x) 字符串,定长
 * varchar(x) 字符串,变长
 **/

#define INT_TYPE ("int")
#define CHAR_TYPE ("char")
#define VARCHAR_TYPE ("varchar")

size_t TypeSize(const string& typeName); // 该类型单个元素的大小
bool IsTypeFixed(const string& typeName);

#endif // _TYPE_INFO_H