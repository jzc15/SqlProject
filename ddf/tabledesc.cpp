#include "tabledesc.h"
#include "record.h"
#include <disk/common.h>
#include <cassert>

TableDesc::TableDesc(DBDesc* dd, const string& tableName, const string& storagePath)
    : dd(dd), tableName(tableName), disk_filename(path_join(storagePath, tableName + ".data"))
{
    cols.clear();
}

TableDesc::TableDesc(DBDesc* dd, const string& tableName, const string& storagePath, const Json& info)
    : dd(dd), tableName(tableName), disk_filename(path_join(storagePath, tableName + ".data"))
{
    assert(info.is_array());
    cols.clear();

    for(auto col: info.array_items())
    {
        cols.push_back(make_shared<ColDesc>(this, col));
    }
    Finalize();
}

TableDesc::~TableDesc()
{

}

ColDesc::ptr TableDesc::CreateColumn(const string& columnName, const string& typeName, size_t length, bool allow_null)
{
    auto p = make_shared<ColDesc>(this, columnName, typeName, length, allow_null);
    cols.push_back(p);
    return p;
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
    primary_key_idx = -1;
    for(int i = 0; i < (int)cols.size(); i ++)
        if (cols[i]->is_primary)
            primary_key_idx = i;
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

int TableDesc::PrimaryKeyIdx()
{
    return primary_key_idx;
}

ColDesc::ptr TableDesc::Column(const string& columnName)
{
    assert(columnPtr.find(columnName) != columnPtr.end());
    return columnPtr[columnName];
}

ColDesc::ptr TableDesc::Column(int index)
{
    assert(0 <= index && index < (int)cols.size());
    return cols[index];
}

bool TableDesc::IsColumnExists(const string& columnName)
{
    return columnPtr.find(columnName) != columnPtr.end();
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
