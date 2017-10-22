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
}

shared_ptr<Record> TableDescription::NewRecord()
{
    return make_shared<Record>(this);
}

int TableDescription::ColumnIndex(const string& columnName)
{
    if (columnIndex.find(columnName) == columnIndex.end())
        assert(false);
    return columnIndex[columnName];
}

int TableDescription::ColumnSize()
{
    return cols.size();
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