#ifndef _RECORD_H
#define _RECORD_H

#include <vector>
#include <memory>
#include "tabledesc.h"
#include "disk/common.h"

using namespace std;

// 描诉一个记录
// 注意：使用Record的时候要求td指向的TableDescription对象必须存在，不能被销毁
class Record
{
public:
    typedef shared_ptr<Record> ptr;

    TableDesc* td;

    Record(TableDesc* td);
    ~Record();

    // 生成序列化的数据
    data_t Generate();
    // 恢复
    void Recover(data_t data);
    // 复制
    static Record::ptr Clone(Record::ptr record);

    data_t PrimaryKey();

    void SetValue(const string& columnName, data_t data);
    void SetValue(int columnIndex, data_t data);

    data_t GetValue(const string& columnName);
    data_t GetValue(int columnIndex);

    void SetInt(const string& columnName, int value);
    void SetInt(int columnIndex, int value);
    int GetInt(const string& columnName);
    int GetInt(int columnIndex);

    void SetFloat(const string& columnName, float value);
    void SetFloat(int columnIndex, float value);
    float GetFloat(const string& columnName);
    float GetFloat(int columnIndex);

    void SetString(const string& columnName, const string& value);
    void SetString(int columnIndex, const string& value);
    string GetString(const string& columnName);
    string GetString(int columnIndex);

    void SetNull(const string& columnName);
    void SetNull(int columnIndex);
    bool IsNull(const string& columnName);
    bool IsNull(int columnIndex);

    void Output();

private:
    vector<data_t> values; // 每列的值
};

#endif // _RECORD_H