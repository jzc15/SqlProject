#include "record.h"
#include "typeinfo.h"
#include <memory>
#include <cstring>
#include <assert.h>
#include <iostream>

using namespace std;

Record::Record(TableDesc* td)
    : td(td)
{
    values.resize(td->ColumnCount());
    for(auto& v: values)
    {
        v = Json(nullptr);
    }
}

Record::~Record()
{
    
}

data_t Record::Generate()
{
    size_t size = 0;
    const int column_count = td->ColumnCount();
    const int unfixed_column_count = td->UnfixedColumnCount();
    int fixed_data_size = 0;

    size += 4;
    for(int i = 0; i < column_count; i ++)
    {
        ColDesc::ptr col = td->Column(i);
        if (col->fixed) {
            size += col->size;
            fixed_data_size += col->size;
        } else { // FIXME 目前只有varchar和text时变长
            size += values[i].string_value().length();
        }
    }
    size += 2 + (column_count+7)/8 + 2 + 2*unfixed_column_count;

    data_t data = alloc_data(size);
    uint8* ptr = data->data();
    ptr[0] = 0; // 状态A FIXME
    ptr[1] = 0; // 状态B
    *(ushort*)(ptr+2) = fixed_data_size + 4;
    size_t offset = 4;
    // 定长数据
    for(int i = 0; i < column_count; i ++)
    {
        ColDesc::ptr col = td->Column(i);
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
        ColDesc::ptr col = td->Column(i);
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

    return data;
}

void Record::Recover(data_t data)
{
    const size_t size = data->size();
    const uint8* ptr = data->data();
    const int column_count = td->ColumnCount();
    const int fixed_column_count = td->FixedColumnCount();
    const int unfixed_column_count = td->UnfixedColumnCount();

    const uint8 statusA = ptr[0];
    const uint8 statusB = ptr[1];
    const ushort fixed_size = *(ushort*)(ptr+2);
    assert(fixed_size == td->FixedDataSize() + 4);
    
    size_t offset = 4;
    for(int i = 0; i < column_count; i ++)
    {
        ColDesc::ptr col = td->Column(i);
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
        ColDesc::ptr col = td->Column(i);
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

data_t Record::GetValue(const string& columnName)
{
    return GetValue(td->ColumnIndex(columnName));
}
data_t Record::GetValue(int columnIndex)
{
    if (IsNull(columnIndex)) return nullptr;

    ColDesc::ptr col = td->Column(columnIndex);
    switch(col->typeEnum)
    {
    case INT_ENUM:
        {
            data_t data = alloc_data(4);
            *(int*)(data->data()) = values[columnIndex].int_value();
            return data;
        }
    case CHAR_ENUM: case VARCHAR_ENUM:
        {
            string x = values[columnIndex].string_value();
            data_t data = alloc_data(x.length());
            memcpy(data->data(), x.c_str(), x.length());
            return data;
        }
    default:
        assert(false);
    }
}

void Record::Output()
{
    for(int i = 0; i < (int)td->cols.size(); i ++)
    {
        ColDesc::ptr col = td->cols[i];
        cout << col->columnName << " : ";
        if (IsNull(i))
        {
            cout << "NULL";
        } else {
            switch(col->typeEnum)
            {
            case INT_ENUM:
                {
                    cout << values[i].int_value();
                    break;
                }
            case CHAR_ENUM: case VARCHAR_ENUM:
                {
                    cout << values[i].string_value();
                    break;
                }
            default:
                assert(false);
            }
        }
        cout << " | ";
    }
    cout << endl;
}
