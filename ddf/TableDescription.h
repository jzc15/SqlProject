#ifndef _TD_H
#define _TD_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <json11.hpp>
#include "ColumnDescription.h"
#include <common/RecordBinary.h>

using namespace std;
using namespace json11;

class Record;

// 表描诉类
class TableDescription
{
public:
    typedef shared_ptr<TableDescription> ptr;

    string databaseName;
    string tableName;
    vector<ColumnDescription::ptr> cols;

    TableDescription(const string& databaseName, const string& tableName);
    TableDescription(const string& databaseName, const string& tableName, const Json& info);
    ~TableDescription();

    // 创建新列，并不会修改数据，表结构应该在一开始就定好
    void CreateColumn(const string& columnName, const string& typeName, size_t length = 1);

    // 当表结构不会再修改的时候调用
    void Finalize();

    // 创建新记录
    shared_ptr<Record> NewRecord();

    // 恢复记录
    shared_ptr<Record> RecoverRecord(RecordBinary data);

    int ColumnIndex(const string& columnName);

    int ColumnCount();
    int FixedColumnCount();
    int UnfixedColumnCount();
    size_t FixedDataSize();

    ColumnDescription::ptr Column(const string& columnName);
    ColumnDescription::ptr Column(int index);

    Json Dump();

private:
    int fixed_column_count;
    int unfixed_column_count;
    size_t fixed_data_size;
    map<string, int> columnIndex;
    map<string, ColumnDescription::ptr> columnPtr;
};

#endif // _TD_H