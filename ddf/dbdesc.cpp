#include "dbdesc.h"
#include <cassert>
#include <json11.hpp>
#include "config.h"
#include <string>
#include <fstream>
#include <streambuf>

using namespace std;
using namespace json11;

DBDesc::DBDesc(const string& databaseName, const string& storagePath)
    : databaseName(databaseName), storagePath(storagePath), ddf_filename(path_join(storagePath, DDF_FILENAME))
{
    tables.clear();

    ifstream fd(ddf_filename);
    if (!fd) return;

    string str((std::istreambuf_iterator<char>(fd)), std::istreambuf_iterator<char>());
    string error_msg;

    Json info = Json::parse(str, error_msg);
    assert(error_msg == "");

    for(auto iter: info.object_items())
    {
        tables[iter.first] = make_shared<TableDesc>(this, iter.first, storagePath, iter.second);
    }
}

DBDesc::~DBDesc()
{
    Save();
}

TableDesc::ptr DBDesc::CreateTable(const string& tableName)
{
    assert(tables.find(tableName) == tables.end());
    auto ptr = tables[tableName] = make_shared<TableDesc>(this, tableName, storagePath);
    return ptr;
}

TableDesc::ptr DBDesc::DropTable(const string& tableName)
{
    assert(tables.find(tableName) != tables.end());
    auto ptr = tables[tableName];
    tables.erase(tables.find(tableName));
    return ptr;
}

TableDesc::ptr DBDesc::SearchTable(const string& tableName)
{
    assert(tables.find(tableName) != tables.end());
    return tables[tableName];
}

vector<string> DBDesc::TableList()
{
    vector<string> rst;
    for(auto it = tables.begin(); it != tables.end(); it ++)
    {
        rst.push_back(it->first);
    }
    return rst;
}

void DBDesc::Save()
{
    ofstream fd(ddf_filename);
    assert(fd);

    Json::object ddf;
    for(auto iter: tables)
    {
        ddf[iter.first] = iter.second->Dump();
    }

    fd << Json(ddf).dump();
    fd.close();
}