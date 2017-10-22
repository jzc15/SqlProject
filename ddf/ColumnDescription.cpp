#include "ColumnDescription.h"
#include <assert.h>
#include "common/TypeInfo.h"

using namespace std;

ColumnDescription::ColumnDescription(const string& columnName, const string& typeName, size_t length)
{
    this->columnName = columnName;
    this->typeName = typeName;
    this->length = length;

    normalize();
}

ColumnDescription::ColumnDescription(const Json& info)
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

ColumnDescription::~ColumnDescription()
{
    // nothing
}

void ColumnDescription::normalize()
{
    std::transform(typeName.begin(), typeName.end(), typeName.begin(), ::tolower);
    fixed = IsTypeFixed(typeName);
    size = TypeSize(typeName)*length;
}

Json ColumnDescription::Dump()
{
    Json::object obj;
    obj["col_name"] = columnName;
    obj["type_name"] = typeName;
    obj["length"] = (int)length;
    return obj;
}