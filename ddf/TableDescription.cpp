#include "TableDescription.h"
#include "common/Record.h"
#include <assert.h>

TableDescription::TableDescription(const string& databaseName, const string& tableName)
{
    this->databaseName = databaseName;
    this->tableName = tableName;
    cols.clear();
}

TableDescription::TableDescription(const string& databaseName, const string& tableName, const Json& info)
{
    assert(info.is_array());

    this->databaseName = databaseName;
    this->tableName = tableName;
    cols.clear();

    for(auto col: info.array_items())
    {
        cols.push_back(make_shared<ColumnDescription>(col));
    }
    Finalize();
}

TableDescription::~TableDescription()
{

}

void TableDescription::CreateColumn(const string& columnName, const string& typeName, size_t length)
{
    cols.push_back(make_shared<ColumnDescription>(columnName, typeName, length));
}

void TableDescription::Finalize()
{
    columnIndex.clear();
    columnPtr.clear();
    for(int i = 0; i < (int)cols.size(); i ++)
    {
        columnIndex[cols[i]->columnName] = i;
        columnPtr[cols[i]->columnName] = cols[i];
    }
    fixed_column_count = 0;
    unfixed_column_count = 0;
    fixed_data_size = 0;
    for(auto col: cols)
    {
        if (col->fixed) fixed_column_count ++;
        else unfixed_column_count ++;

        if (col->fixed) fixed_data_size += col->size;
    }
}

shared_ptr<Record> TableDescription::NewRecord()
{
    return make_shared<Record>(this);
}

shared_ptr<Record> TableDescription::RecoverRecord(RecordBinary data)
{
    auto ptr = make_shared<Record>(this);
    ptr->Recover(data);
    return ptr;
}

int TableDescription::ColumnIndex(const string& columnName)
{
    if (columnIndex.find(columnName) == columnIndex.end())
        assert(false);
    return columnIndex[columnName];
}

int TableDescription::ColumnCount()
{
    return cols.size();
}

int TableDescription::FixedColumnCount()
{
    return fixed_column_count;
}

int TableDescription::UnfixedColumnCount()
{
    return unfixed_column_count;
}

size_t TableDescription::FixedDataSize()
{
    return fixed_data_size;
}

ColumnDescription::ptr TableDescription::Column(const string& columnName)
{
    if (columnPtr.find(columnName) == columnPtr.end())
        assert(false);
    return columnPtr[columnName];
}

ColumnDescription::ptr TableDescription::Column(int index)
{
    assert(0 <= index && index < cols.size());
    return cols[index];
}

Json TableDescription::Dump()
{
    Json::array arr;
    for(auto col: cols)
    {
        arr.push_back(col->Dump());
    }
    return arr;
}