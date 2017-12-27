#include "record.h"
#include "typeinfo.h"
#include <memory>
#include <cstring>
#include <assert.h>
#include <iostream>

using namespace std;

Record::ptr Record::Clone(Record::ptr record)
{
    Record::ptr p = make_shared<Record>(record->td);
    for(int i = 0; i < (int)record->values.size(); i ++)
    {
        p->values[i] = clone(record->values[i]);
    }
    return p;
}

Record::Record(TableDesc* td)
    : td(td)
{
    values.resize(td->ColumnCount());
    for(auto& v: values)
    {
        v = nullptr;
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
        } else {
            if (values[i] != nullptr) size += values[i]->size();
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
            if (values[i] != nullptr) {
                switch(col->typeEnum) {
                    case INT_ENUM:
                        *(int*)(ptr+offset) = *(int*)(values[i]->data());
                        break;
                    case FLOAT_ENUM:
                        *(float*)(ptr+offset) = *(float*)(values[i]->data());
                        break;
                    case CHAR_ENUM:
                        {
                            memcpy(ptr+offset, values[i]->data(), values[i]->size());
                        }
                        break;
                    case DATE_ENUM:
                        *(time_t*)(ptr+offset) = *(time_t*)(values[i]->data());
                        break;
                    default:
                        assert(false);
                        break;
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
        if (values[i] == nullptr)
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
            if (values[i] != nullptr) {
                memcpy(ptr+offset, values[i]->data(), values[i]->size());
                offset += values[i]->size();
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
                    values[i] = int_data(*(int*)(ptr+offset));
                    break;
                case FLOAT_ENUM:
                    values[i] = float_data(*(float*)(ptr+offset));
                    break;
                case CHAR_ENUM:
                    values[i] = string_data(string((char*)(ptr+offset), col->length));
                    break;
                case DATE_ENUM:
                    values[i] = time_data(*(time_t*)(ptr+offset));
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
            values[i] = nullptr;
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
                values[i] = string_data(string((char*)(ptr+offset), end-offset));
            offset = end;
            id ++;
        }
    }
    assert(offset == size);
}

data_t Record::PrimaryKey()
{
    data_t idxs = td->PrimaryIdxs();
    data_t key = alloc_data(0);
    for(int i = 0; i < (int)idxs->size(); i ++)
    {
        data_t value = values[idxs->at(i)];
        for(int j = 0; j < (int)value->size(); j ++)
            key->push_back(value->at(j));
    }
    return key;
}

void Record::SetValue(const string& columnName, data_t data)
{
    SetValue(td->ColumnIndex(columnName), data);
}

void Record::SetValue(int columnIndex, data_t data)
{
    values[columnIndex] = data = clone(data);
    if (data == nullptr) return;

    ColDesc::ptr col = td->Column(columnIndex);
    switch(col->typeEnum)
    {
    case INT_ENUM: case FLOAT_ENUM:
        assert(data->size() == 4);
        break;
    case CHAR_ENUM:
        if ((int)data->size() > col->length) data->resize(col->length);
        while((int)data->size() < col->length) data->push_back(' ');
        break;
    case VARCHAR_ENUM:
        data->resize(min(data->size(), col->length));
        break;
    case DATE_ENUM:
        assert(data->size() == sizeof(time_t));
        break;
    default:
        assert(false);
    }
}

data_t Record::GetValue(const string& columnName)
{
    return GetValue(td->ColumnIndex(columnName));
}
data_t Record::GetValue(int columnIndex)
{
    return values[columnIndex];
}

void Record::SetInt(const string& columnName, int value)
{
    SetInt(td->ColumnIndex(columnName), value);
}
void Record::SetInt(int columnIndex, int value)
{
    values[columnIndex] = int_data(value);
}
int Record::GetInt(const string& columnName)
{
    return GetInt(td->ColumnIndex(columnName));
}
int Record::GetInt(int columnIndex)
{
    return *(int*)(values[columnIndex]->data());
}

void Record::SetFloat(const string& columnName, float value)
{
    SetFloat(td->ColumnIndex(columnName), value);
}
void Record::SetFloat(int columnIndex, float value)
{
    values[columnIndex] = float_data(value);
}
float Record::GetFloat(const string& columnName)
{
    return GetFloat(td->ColumnIndex(columnName));
}
float Record::GetFloat(int columnIndex)
{
    return *(float*)(values[columnIndex]->data());
}

void Record::SetString(const string& columnName, const string& value)
{
    return SetString(td->ColumnIndex(columnName), value);
}
void Record::SetString(int columnIndex, const string& value)
{
    values[columnIndex] = string_data(value);
}
string Record::GetString(const string& columnName)
{
    return GetString(td->ColumnIndex(columnName));
}
string Record::GetString(int columnIndex)
{
    return string((char*)values[columnIndex]->data(), values[columnIndex]->size());
}

void Record::SetNull(const string& columnName)
{
    return SetNull(td->ColumnIndex(columnName));
}
void Record::SetNull(int columnIndex)
{
    values[columnIndex] = nullptr;
}
bool Record::IsNull(const string& columnName)
{
    return IsNull(td->ColumnIndex(columnName));
}
bool Record::IsNull(int columnIndex)
{
    return values[columnIndex] == nullptr;
}

void Record::Output()
{
    for(int i = 0; i < (int)td->cols.size(); i ++)
    {
        ColDesc::ptr col = td->cols[i];
        cout << col->columnName << " : ";
        if (values[i] == nullptr)
        {
            cout << "NULL";
        } else {
            switch(col->typeEnum)
            {
            case INT_ENUM:
                {
                    cout << *(int*)(values[i]->data());
                    break;
                }
            case FLOAT_ENUM:
                {
                    cout << *(float*)(values[i]->data());
                    break;
                }
            case CHAR_ENUM: case VARCHAR_ENUM:
                {
                    cout << string((char*)values[i]->data(), values[i]->size());
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
