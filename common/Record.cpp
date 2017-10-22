#include "Record.h"
#include "TypeInfo.h"
#include <memory>
#include <cstring>
#include <assert.h>
#include <iostream>

using namespace std;

Record::Record(TableDescription* td)
{
    this->td = td;
    values.resize(td->ColumnCount());
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
    const int column_count = td->ColumnCount();
    const int unfixed_column_count = td->UnfixedColumnCount();
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
        }
    }
    size += 2 + (column_count+7)/8 + 2 + 2*unfixed_column_count;

    uchar* ptr = new uchar[size]; // FIXME
    memset(ptr, 0, sizeof(uchar)*size);
    ptr[0] = 0; // 状态A FIXME
    ptr[1] = 0; // 状态B
    *(ushort*)(ptr+2) = fixed_data_size + 4;
    size_t offset = 4;
    // 定长数据
    for(int i = 0; i < column_count; i ++)
    {
        ColumnDescription::ptr col = td->Column(i);
        if (col->fixed) {
            if (!values[i].is_null()) {
                switch(col->typeEnum) {
                    case INT_ENUM:
                        *(int*)(ptr+offset) = values[i].int_value();
                        break;
                    case CHAR_ENUM:
                        {
                            string data = values[i].string_value();
                            memset(ptr+offset, ' ', col->length);
                            memcpy(ptr+offset, data.c_str(), min(col->length, data.length()));
                        }
                        break;
                    default:assert(false);break;
                }
            }
            offset += col->size;
        }
    }
    *(ushort*)(ptr+offset) = column_count; // 列数
    offset += 2;
    // null位图
    for(int i = 0; i < column_count; i ++)
    {
        if (values[i].is_null())
        {
            ptr[offset+i/8] |= 1<<(i%8);
        }
    }
    offset += (column_count+7)/8;

    *(ushort*)(ptr+offset) = unfixed_column_count; // 变长列数
    offset += 2;

    size_t unfixed_offset = offset;
    offset += 2*unfixed_column_count;
    for(int i = 0, id = 0; i < column_count; i ++)
    {
        ColumnDescription::ptr col = td->Column(i);
        if (!col->fixed)
        {
            if (!values[i].is_null()) {
                string data = values[i].string_value(); // FIXME 目前只有字符串可以变长
                size_t length = min(data.length(), col->length);
                memcpy(ptr+offset, data.c_str(), length);
                offset += length;
            }
            *(ushort*)(ptr+unfixed_offset+id*2) = offset;
            id ++;
        }
    }

    assert(size == offset);

    return (RecordBinary){ptr, size};
}

void Record::Recover(RecordBinary data)
{
    const size_t size = data.size;
    const uchar* ptr = data.ptr;
    const int column_count = td->ColumnCount();
    const int fixed_column_count = td->FixedColumnCount();
    const int unfixed_column_count = td->UnfixedColumnCount();

    const uchar statusA = ptr[0];
    const uchar statusB = ptr[1];
    const ushort fixed_size = *(ushort*)(ptr+2);
    assert(fixed_size == td->FixedDataSize() + 4);
    
    size_t offset = 4;
    for(int i = 0; i < column_count; i ++)
    {
        ColumnDescription::ptr col = td->Column(i);
        if (col->fixed) {
            switch(col->typeEnum) {
                case INT_ENUM:
                    values[i] = Json(*(int*)(ptr+offset));
                    break;
                case CHAR_ENUM:
                    values[i] = Json(string((char*)(ptr+offset), col->length));
                    break;
                default: assert(false); break;
            }
            offset += col->size;
        }
    }
    assert(*(ushort*)(ptr+offset) == column_count);
    offset += 2;
    const size_t null_offset = offset;
    for(int i = 0; i < column_count; i ++)
    {
        if ((ptr[null_offset+i/8]>>(i%8))&1) {
            values[i] = Json(nullptr);
        }
    }
    offset += (column_count+7)/8;
    assert(*(ushort*)(ptr+offset) == unfixed_column_count);
    offset += 2;
    const size_t unfixed_offset = offset;
    offset += unfixed_column_count*2;
    for(int i = 0, id = 0; i < column_count; i ++)
    {
        ColumnDescription::ptr col = td->Column(i);
        if (!col->fixed) {
            ushort end = *(ushort*)(ptr+unfixed_offset+id*2);
            if (!((ptr[null_offset+i/8]>>(i%8))&1))
                values[i] = Json(string((char*)(ptr+offset), end-offset));
            offset = end;
            id ++;
        }
    }
    assert(offset == size);
}

void Record::SetNull(const string& columnName)
{
    values[td->ColumnIndex(columnName)] = Json(nullptr);
}
void Record::SetNull(int columnIndex)
{
    values[columnIndex] = Json(nullptr);
}
bool Record::IsNull(const string& columnName)
{
    return values[td->ColumnIndex(columnName)].is_null();
}
bool Record::IsNull(int columnIndex)
{
    return values[columnIndex].is_null();
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
int Record::GetInt(const string& columnName)
{
    return values[td->ColumnIndex(columnName)].int_value();
}
int Record::GetInt(int columnIndex)
{
    return values[columnIndex].int_value();
}

// char
void Record::SetChar(const string& columnName, char value)
{
    values[td->ColumnIndex(columnName)] = Json(string(&value, 1));
}
void Record::SetChar(int columnIndex, char value)
{
    values[columnIndex] = Json(string(&value, 1));
}
char Record::GetChar(const string& columnName)
{
    return values[td->ColumnIndex(columnName)].string_value()[0];
}
char Record::GetChar(int columnIndex)
{
    return values[columnIndex].string_value()[0];
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
string Record::GetString(const string& columnName)
{
    return values[td->ColumnIndex(columnName)].string_value();
}
string Record::GetString(int columnIndex)
{
    return values[columnIndex].string_value();
}
