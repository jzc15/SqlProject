#ifndef _TYPE_INFO_H
#define _TYPE_INFO_H

#include <disk/common.h>
#include <string>

using namespace std;

// 类型信息，类型均用小写字符串表示
/**
 * 目前支持的类型：
 * int 整数,定长
 * char 字符,定长
 * char(x) 字符串,定长
 * varchar(x) 字符串,变长
 * date 日期，定长，存时间戳，int
 * float 浮点数，定长，双精度
 **/

#define INT_TYPE ("int")
#define CHAR_TYPE ("char")
#define VARCHAR_TYPE ("varchar")
#define DATE_TYPE ("date") // DATE储存为int
#define FLOAT_TYPE ("float")
#define DECIMAL_TYPE ("decimal") // 定点数，两个int，9位

enum type_t {
    INT_ENUM = 0,
    CHAR_ENUM,
    VARCHAR_ENUM,
    DATE_ENUM,
    FLOAT_ENUM,
    DECIMAL_ENUM,
};

size_t type_size(const string& typeName); // 该类型单个元素的大小
bool is_type_fixed(const string& typeName);
type_t type_enum(const string& typeName);
string type_name(type_t type_enum);

// return a - b;
int compare(type_t type, data_t data_a, data_t data_b);

string stringify(type_t type, data_t data);

#endif // _TYPE_INFO_H