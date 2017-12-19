#include "tabledesc.h"
#include "record.h"
#include <assert.h>

TableDesc::TableDesc(const string& databaseName, const string& tableName, const string& storagePath)
    : databaseName(databaseName), tableName(tableName), disk_filename(path_join(storagePath, tableName + ".data"))
{
    cols.clear();
}

TableDesc::TableDesc(const string& databaseName, const string& tableName, const string& storagePath, const Json& info)
    : databaseName(databaseName), tableName(tableName), disk_filename(path_join(storagePath, tableName + ".data"))
{
    assert(info.is_array());
    cols.clear();

    for(auto col: info.array_items())
    {
        cols.push_back(make_shared<ColDesc>(col));
    }
    Finalize();
}

TableDesc::~TableDesc()
{

}

void TableDesc::CreateColumn(const string& columnName, const string& typeName, size_t length)
{
    cols.push_back(make_shared<ColDesc>(columnName, typeName, length));
}

void TableDesc::Finalize()
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

Record::ptr TableDesc::NewRecord()
{
    return make_shared<Record>(this);
}

Record::ptr TableDesc::RecoverRecord(data_t data)
{
    auto ptr = make_shared<Record>(this);
    ptr->Recover(data);
    return ptr;
}

int TableDesc::ColumnIndex(const string& columnName)
{
    if (columnIndex.find(columnName) == columnIndex.end())
        assert(false);
    return columnIndex[columnName];
}

int TableDesc::ColumnCount()
{
    return cols.size();
}

int TableDesc::FixedColumnCount()
{
    return fixed_column_count;
}

int TableDesc::UnfixedColumnCount()
{
    return unfixed_column_count;
}

size_t TableDesc::FixedDataSize()
{
    return fixed_data_size;
}

ColDesc::ptr TableDesc::Column(const string& columnName)
{
    if (columnPtr.find(columnName) == columnPtr.end())
        assert(false);
    return columnPtr[columnName];
}

ColDesc::ptr TableDesc::Column(int index)
{
    assert(0 <= index && index < cols.size());
    return cols[index];
}

Json TableDesc::Dump()
{
    Json::array arr;
    for(auto col: cols)
    {
        arr.push_back(col->Dump());
    }
    return arr;
}
