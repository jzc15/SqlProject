#include "coldesc.h"
#include <assert.h>
#include "typeinfo.h"

using namespace std;

ColDesc::ColDesc(const string& columnName, const string& typeName, size_t length)
{
    this->columnName = columnName;
    this->typeName = typeName;
    this->length = length;

    normalize();
}

ColDesc::ColDesc(const Json& info)
{
    assert(info.is_object());
    assert(info["col_name"].is_string());
    assert(info["type_name"].is_string());
    assert(info["length"].is_number());

    columnName = info["col_name"].string_value();
    typeName = info["type_name"].string_value();
    length = info["length"].int_value();

    normalize();
}

ColDesc::~ColDesc()
{
    // nothing
}

void ColDesc::normalize()
{
    std::transform(typeName.begin(), typeName.end(), typeName.begin(), ::tolower);
    length = max((int)length, 1);
    fixed = is_type_fixed(typeName);
    size = type_size(typeName)*length;
    typeEnum = type_enum(typeName);
}

Json ColDesc::Dump()
{
    Json::object obj;
    obj["col_name"] = columnName;
    obj["type_name"] = typeName;
    obj["length"] = (int)length;
    return obj;
}