#include "coldesc.h"
#include "tabledesc.h"
#include "dbdesc.h"
#include "typeinfo.h"
#include <cassert>
#include <disk/common.h>

using namespace std;

ColDesc::ColDesc(TableDesc* td, const string& columnName, const string& typeName, size_t length, bool allow_null)
{
    this->td = td;

    this->columnName = columnName;
    this->typeName = typeName;
    this->allow_null = allow_null;
    if (typeName == CHAR_TYPE || typeName == VARCHAR_TYPE)
        this->length = length;
    else this->length = 1;
    if (typeName == INT_TYPE)
        this->display_length = length;
    else this->display_length = 1;
    this->indexed = false;
    this->is_primary = false;
    this->is_foreign_key = false;

    normalize();
}

ColDesc::ColDesc(TableDesc* td, const Json& info)
{
    this->td = td;

    assert(info.is_object());
    assert(info["col_name"].is_string());
    assert(info["type_name"].is_string());
    assert(info["length"].is_number());
    assert(info["display_length"].is_number());
    assert(info["allow_null"].is_bool());
    assert(info["indexed"].is_bool());
    assert(info["is_primary"].is_bool());
    assert(info["is_foreign_key"].is_bool());

    columnName = info["col_name"].string_value();
    typeName = info["type_name"].string_value();
    length = info["length"].int_value();
    display_length = info["display_length"].int_value();
    allow_null = info["allow_null"].bool_value();
    indexed = info["indexed"].bool_value();
    is_primary = info["is_primary"].bool_value();
    is_foreign_key = info["is_foreign_key"].bool_value();
    if (is_foreign_key)
    {
        foreign_tb_name = info["foreign_tb_name"].string_value();
        foreign_col_name = info["foreign_col_name"].string_value();
    }

    normalize();
}

ColDesc::~ColDesc()
{
    // nothing
}

int ColDesc::Length()
{
    if (typeName == CHAR_TYPE || typeName == VARCHAR_TYPE)
        return length;
    if (typeName == INT_TYPE)
        return display_length;
    return -1;
}

void ColDesc::SetIndexed()
{
    this->indexed = true;
}

void ColDesc::SetPrimary()
{
    this->is_primary = true;
    this->allow_null = false;
}

void ColDesc::SetForeignKey(string foreign_tb_name, string foreign_col_name)
{
    this->is_foreign_key = true;
    this->foreign_tb_name = foreign_tb_name;
    this->foreign_col_name = foreign_col_name;
}

void ColDesc::normalize()
{
    std::transform(typeName.begin(), typeName.end(), typeName.begin(), ::tolower);
    length = max((int)length, 1);
    fixed = is_type_fixed(typeName);
    size = type_size(typeName)*length;
    typeEnum = type_enum(typeName);
}

string ColDesc::IndexFilename()
{
    return path_join(td->dd->storagePath, td->tableName + "." + columnName + ".index");
}

string ColDesc::PrimaryFilename()
{
    return path_join(td->dd->storagePath, td->tableName + "." + columnName + ".primary.index");
}

Json ColDesc::Dump()
{
    Json::object obj;
    obj["col_name"] = columnName;
    obj["type_name"] = typeName;
    obj["length"] = (int)length;
    obj["display_length"] = (int)display_length;
    obj["allow_null"] = allow_null;
    obj["indexed"] = indexed;
    obj["is_primary"] = is_primary;
    obj["is_foreign_key"] = is_foreign_key;
    obj["foreign_tb_name"] = foreign_tb_name;
    obj["foreign_col_name"] = foreign_col_name;
    return obj;
}
