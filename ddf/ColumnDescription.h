#ifndef _CD_H
#define _CD_H

#include <algorithm>
#include <string>
#include <json11.hpp>
#include <map>
#include <memory>

using namespace std;
using namespace json11;

// 列描述类
class ColumnDescription
{
public:
    typedef shared_ptr<ColumnDescription> ptr;

    string columnName;
    string typeName; // 小写
    size_t length; // 长度，对于某些类型时长度限制(varchar)，对于某些类型时指定长度(char)，对于int等定长数据应该恒为1
    size_t size; // 对于定长数据，占多少*字节*的空间
    bool fixed; // 是否定长

    ColumnDescription(const string& columnName, const string& typeName, size_t length);
    ColumnDescription(const Json& info); // 从json中载入
    ~ColumnDescription();

    Json Dump();

private:
    void normalize();
};

#endif // _CD_H