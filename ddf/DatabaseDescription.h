#ifndef _DD_H
#define _DD_H

#include <string>
#include <map>
#include <memory>

#include "TableDescription.h"

using namespace std;

// 数据库描诉类
class DatabaseDescription
{
public:
    typedef shared_ptr<DatabaseDescription> ptr;

    string databaseName;

    DatabaseDescription(const string& databaseName);
    ~DatabaseDescription();

    // 创建新表，初始时没有任何列，只会影响表描诉json文件，并不会创建表文件
    TableDescription::ptr CreateTable(const string& tableName);
    // 删除表，并不会删除表文件，返回被删除的表描诉类
    TableDescription::ptr DropTable(const string& tableName);
    // 查找表
    TableDescription::ptr SearchTable(const string& tableName);

    void Save();

private:
    map<string, TableDescription::ptr> tables;
};

#endif // _DD_H