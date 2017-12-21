#include "typeinfo.h"
#include <map>
#include <cassert>

using namespace std;

static map<string, size_t> typeSize = {
    {INT_TYPE, 4},
    {CHAR_TYPE, 1},
    {VARCHAR_TYPE, 1},
    {DATE_TYPE, 4},
    {FLOAT_TYPE, 8}
};
size_t type_size(const string& typeName)
{
    assert(typeSize.find(typeName) != typeSize.end());
    return typeSize[typeName];
}

static map<string, bool> typeFixed = {
    {INT_TYPE, true},
    {CHAR_TYPE, true},
    {VARCHAR_TYPE, false},
    {DATE_TYPE, true},
    {FLOAT_TYPE, true}
};
bool is_type_fixed(const string& typeName)
{
    assert(typeFixed.find(typeName) != typeFixed.end());
    return typeFixed[typeName];
}

static map<string, int> typeEnum = {
    {INT_TYPE, INT_ENUM},
    {CHAR_TYPE, CHAR_ENUM},
    {VARCHAR_TYPE, VARCHAR_ENUM},
    {DATE_TYPE, DATE_ENUM},
    {FLOAT_TYPE, FLOAT_ENUM}
};
type_t type_enum(const string& typeName)
{
    assert(typeEnum.find(typeName) != typeEnum.end());
    return (type_t)typeEnum[typeName];
}

static map<int, string> typeName = {
    {INT_ENUM, INT_TYPE},
    {CHAR_ENUM, CHAR_TYPE},
    {VARCHAR_ENUM, VARCHAR_TYPE},
    {DATE_ENUM, DATE_TYPE},
    {FLOAT_ENUM, FLOAT_TYPE}
};
string type_name(type_t type_enum)
{
    assert(typeName.find((int)type_enum) != typeName.end());
    return typeName[type_enum];
}

int compare(type_t type_a, data_t data_a, type_t type_b, data_t data_b)
{
    if (type_a == INT_ENUM)
    {
        assert(type_b == INT_ENUM);
        return *(int*)(data_a->data()) - *(int*)(data_b->data());
    } else if (type_a == CHAR_ENUM || type_a == VARCHAR_ENUM)
    {
        assert(type_b == CHAR_ENUM || type_b == VARCHAR_ENUM);
        for(int i = 0; i < (int)data_a->size() || i < (int)data_b->size(); i ++)
        {
            if (i == (int)data_b->size()) return -1;
            if (i == (int)data_a->size()) return 1;
            if (data_a->data()[i] != data_b->data()[i]) return data_a->data()[i] - data_b->data()[i];
        }
        return 0;
    } else if (type_a == DATE_ENUM)
    {
        assert(false);
    } else if (type_a == FLOAT_ENUM)
    {
        assert(type_b == FLOAT_ENUM);
        return *(float*)(data_a->data()) - *(float*)(data_b->data());
    } else {
        assert(false);
    }
}