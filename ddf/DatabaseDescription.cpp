#include "DatabaseDescription.h"
#include <assert.h>
#include <json11.hpp>
#include "common/config.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <sys/stat.h>

using namespace std;
using namespace json11;

DatabaseDescription::DatabaseDescription(const string& databaseName)
{
    this->databaseName = databaseName;
    tables.clear();

    ifstream fd(databaseName + "/" + DDF_FILENAME);
    if (!fd) return;

    string str((std::istreambuf_iterator<char>(fd)), std::istreambuf_iterator<char>());
    string error_msg;

    Json info = Json::parse(str, error_msg);
    assert(error_msg == "");

    for(auto iter: info.object_items())
    {
        tables[iter.first] = make_shared<TableDescription>(databaseName, iter.first, iter.second);
    }
}

DatabaseDescription::~DatabaseDescription()
{
    Save();
}

TableDescription::ptr DatabaseDescription::CreateTable(const string& tableName)
{
    assert(tables.find(tableName) == tables.end());
    auto ptr = tables[tableName] = make_shared<TableDescription>(databaseName, tableName);
    return ptr;
}

TableDescription::ptr DatabaseDescription::DropTable(const string& tableName)
{
    assert(tables.find(tableName) != tables.end());
    auto ptr = tables[tableName];
    tables.erase(tables.find(tableName));
    return ptr;
}

TableDescription::ptr DatabaseDescription::SearchTable(const string& tableName)
{
    assert(tables.find(tableName) != tables.end());
    return tables[tableName];
}

vector<string> DatabaseDescription::TableList()
{
    vector<string> rst;
    for(auto it = tables.begin(); it != tables.end(); it ++)
    {
        rst.push_back(it->first);
    }
    return rst;
}

void DatabaseDescription::Save()
{
    mkdir(databaseName.c_str(), 0775);
    ofstream fd(databaseName + "/" + DDF_FILENAME);
    assert(fd);

    Json::object ddf;
    for(auto iter: tables)
    {
        ddf[iter.first] = iter.second->Dump();
    }

    fd << Json(ddf).dump();
    fd.close();
}