#include "typeinfo.h"
#include <map>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

static map<string, size_t> typeSize = {
    {INT_TYPE, 4},
    {CHAR_TYPE, 1},
    {VARCHAR_TYPE, 1},
    {DATE_TYPE, sizeof(time_t)},
    {FLOAT_TYPE, 4},
    {DECIMAL_TYPE, 8}
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
    {FLOAT_TYPE, true},
    {DECIMAL_TYPE, true}
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
    {FLOAT_TYPE, FLOAT_ENUM},
    {DECIMAL_TYPE, DECIMAL_ENUM}
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
    {FLOAT_ENUM, FLOAT_TYPE},
    {DECIMAL_ENUM, DECIMAL_TYPE}
};
string type_name(type_t type_enum)
{
    assert(typeName.find((int)type_enum) != typeName.end());
    return typeName[type_enum];
}

int compare(type_t type, data_t data_a, data_t data_b)
{
    switch(type)
    {
    case INT_ENUM:
        return *(int*)(data_a->data()) - *(int*)(data_b->data());
        break;
    case FLOAT_ENUM:
        return 
            *(float*)(data_a->data()) < *(float*)(data_b->data()) ? -1 : 
            (*(float*)(data_a->data()) == *(float*)(data_b->data()) ? 0 : 1);
        break;
    case DECIMAL_ENUM:
        {
            int* ad = (int*)data_a->data();
            int* bd = (int*)data_b->data();
            return (ad[0] == bd[0]) ? (ad[1] - bd[1]) : (ad[0]-bd[0]);
        }
        break;
    case CHAR_ENUM: case VARCHAR_ENUM:
        for(int i = 0; i < (int)data_a->size() || i < (int)data_b->size(); i ++)
        {
            if (i == (int)data_b->size()) return -1;
            if (i == (int)data_a->size()) return 1;
            if (data_a->data()[i] != data_b->data()[i]) return data_a->data()[i] - data_b->data()[i];
        }
        return 0;
        break;
    default: assert(false);
    }
}

string stringify(type_t type, data_t data)
{
    char buf[1024];
    switch(type)
    {
    case INT_ENUM:
        sprintf(buf, "%d", *(int*)(data->data()));
        break;
    case FLOAT_ENUM:
        sprintf(buf, "%f", *(float*)(data->data()));
        break;
    case CHAR_ENUM: case VARCHAR_ENUM:
        sprintf(buf, "'%s'", string((char*)data->data(), data->size()).c_str());
        break;
    case DATE_ENUM:
        {
            std::stringstream ss;
            ss << std::put_time(std::localtime((time_t*)(data->data())), "%Y-%m-%d");
            return ss.str();
        }
        break;
    case DECIMAL_ENUM:
        sprintf(buf, "%d.%09d", *(int*)(data->data()), *(int*)(data->data()+sizeof(int)));
        break;
    default: assert(false);
    }
    return buf;
}