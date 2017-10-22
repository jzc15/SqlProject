#include "Record.h"

Record::Record(TableDescription* td)
{
    this->td = td;
    values.resize(td->ColumnSize());
    for(auto& v: values)
    {
        v = Json(nullptr);
    }
}

Record::~Record()
{
    
}

RecordBinary Record::Generate()
{
    size_t size = 0;
    const int column_count = td->ColumnSize();
    int unfixed_column_count = 0;
    int fixed_data_size = 0;

    size += 4;
    for(int i = 0; i < column_count; i ++)
    {
        ColumnDescription::ptr col = td->Column(i);
        if (col->fixed) {
            size += col->size;
            fixed_data_size += col->size;
        } else { // FIXME 目前只有varchar和text时变长
            size += values[i].string_value().length();
            unfixed_column_count ++;
        }
    }
    size += 2 + (column_count+7)/8 + 2 + 2*unfixed_column_count;

    uchar* ptr = new uchar[size];
    ptr[0] = 0; // 状态A FIXME
    ptr[1] = 0; // 状态B
}

void Record::SetNull(const string& columnName)
{
    values[td->ColumnIndex(columnName)] = Json(nullptr);
}
void Record::SetNull(int columnIndex)
{
    values[columnIndex] = Json(nullptr);
}

// int
void Record::SetInt(const string& columnName, int value)
{
    values[td->ColumnIndex(columnName)] = Json(value);
}
void Record::SetInt(int columnIndex, int value)
{
    values[columnIndex] = Json(value);
}

// char
void Record::SetChar(const string& columnName, char value)
{
    values[td->ColumnIndex(columnName)] = Json((int)value);
}
void Record::SetChar(int columnIndex, char value)
{
    values[columnIndex] = Json((int)value);
}

// char(10), varchar(200)
void Record::SetString(const string& columnName, const char* value)
{
    values[td->ColumnIndex(columnName)] = Json(value);
}
void Record::SetString(int columnIndex, const char* value)
{
    values[columnIndex] = Json(value);
}