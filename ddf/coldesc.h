#ifndef _CD_H
#define _CD_H

#include <algorithm>
#include <string>
#include <json11.hpp>
#include <map>
#include <memory>
#include <vector>
#include "typeinfo.h"

using namespace std;
using namespace json11;

class TableDesc;

// 列描述类
class ColDesc
{
public:
    typedef shared_ptr<ColDesc> ptr;

    TableDesc* td;

    string columnName;
    string typeName; // 小写
    size_t length; // 长度，对于某些类型时长度限制(varchar)，对于某些类型时指定长度(char)，对于int等定长数据应该恒为1
    size_t display_length; // 展示的长度，对int有效
    bool allow_null;
    bool indexed;
    bool is_oneof_primary;
    bool is_foreign_key;
    string foreign_tb_name;
    string foreign_col_name;
    vector<Json> scope_values;

    type_t typeEnum;
    size_t size; // 对于定长数据，占多少*字节*的空间
    bool fixed; // 是否定长

    bool is_only_primary; // 是否是唯一的主键
    bool has_multi_primary_hash; // 是否有可重的主键
    vector<string> be_refed_tbs; // 被引用的表
    vector<int> be_refed_col_idx; // 被引用的列

    ColDesc(TableDesc* td, const string& columnName, const string& typeName, size_t length, bool allow_null);
    ColDesc(TableDesc* td, const Json& info); // 从json中载入
    ~ColDesc();

    int Length();
    void SetIndexed(bool indexed = true);
    void SetOneOfPrimary();
    void SetForeignKey(string foreign_tb_name, string foreign_col_name);
    void SetScopeValues(const vector<Json>& values);

    string IndexFilename();
    string MultiPrimaryFilename();

    Json Dump();

private:
    void normalize();
};

#endif // _CD_H