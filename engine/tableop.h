#ifndef ENGINE_TABLE_DB_H
#define ENGINE_TABLE_DB_H

#include "context.h"
#include "crudop.h"
#include <ddf/typeinfo.h>

// 类型
class Type
{
public:
    type_t type;
    int length;

    Type(){}
    Type(type_t type, int length = 1): type(type), length(length) {}
};
// 列定义
class ColumnDefine
{
public:
    string col_name;
    Type type;
    bool allow_null;
    ColumnDefine() {}
    ColumnDefine(string col_name, Type type, bool allow_null): col_name(col_name), type(type), allow_null(allow_null) {}
};
// 外链
class ColumnForeign
{
public:
    string col_name;
    string ref_tb_name;
    string ref_col_name;

    ColumnForeign(){}
    ColumnForeign(string col_name, string ref_tb_name, string ref_col_name): col_name(col_name), ref_tb_name(ref_tb_name), ref_col_name(ref_col_name) {}
};
// 域约束
class ScopeLimit
{
public:
    string col_name;
    vector<Value> values;

    ScopeLimit(){}
    ScopeLimit(const string& col_name, const vector<Value>& values): col_name(col_name), values(values) {}
};

void show_tables(Context* ctx);
void create_table(Context* ctx, 
    const string& tb_name,
    const vector<ColumnDefine>& cols,
    const vector<string>& primary_cols,
    const vector<ColumnForeign>& foreign_cols,
    const vector<ScopeLimit>& scope_limits);
void drop_table(Context* ctx, const string& tb_name);
void desc_table(Context* ctx, const string& tb_name);

#endif // ENGINE_TABLE_DB_H