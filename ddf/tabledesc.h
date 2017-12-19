#ifndef _TD_H
#define _TD_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <json11.hpp>
#include "coldesc.h"
#include <disk/common.h>

using namespace std;
using namespace json11;

class Record;

// 表描诉类
class TableDesc
{
public:
    typedef shared_ptr<TableDesc> ptr;

    const string databaseName;
    const string tableName;
    const string disk_filename;

    vector<ColDesc::ptr> cols;

    TableDesc(const string& databaseName, const string& tableName, const string& storagePath);
    TableDesc(const string& databaseName, const string& tableName, const string& storagePath, const Json& info);
    ~TableDesc();

    // 创建新列，并不会修改数据，表结构应该在一开始就定好
    void CreateColumn(const string& columnName, const string& typeName, size_t length = 1);

    // 当表结构不会再修改的时候调用
    void Finalize();

    // 创建新记录
    shared_ptr<Record> NewRecord();

    // 恢复记录
    shared_ptr<Record> RecoverRecord(data_t data);

    int ColumnIndex(const string& columnName);

    int ColumnCount();
    int FixedColumnCount();
    int UnfixedColumnCount();
    size_t FixedDataSize();

    ColDesc::ptr Column(const string& columnName);
    ColDesc::ptr Column(int index);

    Json Dump();

private:
    int fixed_column_count;
    int unfixed_column_count;
    size_t fixed_data_size;
    map<string, int> columnIndex;
    map<string, ColDesc::ptr> columnPtr;
};

#endif // _TD_H