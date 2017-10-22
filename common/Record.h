#ifndef _RECORD_H
#define _RECORD_H

#include <vector>
#include <memory>
#include <json11.hpp>
#include "ddf/TableDescription.h"
#include "RecordBinary.h"

using namespace std;
using namespace json11;

// 描诉一个记录
// 注意：使用Record的时候要求td指向的TableDescription对象必须存在，不能被销毁
class Record
{
public:
    typedef shared_ptr<Record> ptr;

    TableDescription* td;

    Record(TableDescription* td);
    ~Record();

    // 生成序列化的数据
    RecordBinary Generate();
    // 恢复
    void Recover(RecordBinary data);

    void SetNull(const string& columnName);
    void SetNull(int columnIndex);
    bool IsNull(const string& columnName);
    bool IsNull(int columnIndex);

    // int
    void SetInt(const string& columnName, int value);
    void SetInt(int columnIndex, int value);
    int GetInt(const string& columnName);
    int GetInt(int columnIndex);

    // char
    void SetChar(const string& columnName, char value);
    void SetChar(int columnIndex, char value);
    char GetChar(const string& columnName);
    char GetChar(int columnIndex);

    // char(10), varchar(200)
    void SetString(const string& columnName, const char* value);
    void SetString(int columnIndex, const char* value);
    string GetString(const string& columnName);
    string GetString(int columnIndex);

private:
    vector<Json> values; // 每列的值
};

#endif // _RECORD_H