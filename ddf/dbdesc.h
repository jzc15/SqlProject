#ifndef _DD_H
#define _DD_H

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "tabledesc.h"

using namespace std;

// 数据库描诉类
class DBDesc
{
public:
    typedef shared_ptr<DBDesc> ptr;

    const string databaseName;
    const string storagePath;
    const string ddf_filename;

    DBDesc(const string& databaseName, const string& storagePath);
    ~DBDesc();

    // 创建新表，初始时没有任何列，只会影响表描诉json文件，并不会创建表文件
    TableDesc::ptr CreateTable(const string& tableName);
    // 删除表，并不会删除表文件，返回被删除的表描诉类
    TableDesc::ptr DropTable(const string& tableName);
    // 查找表
    TableDesc::ptr SearchTable(const string& tableName);
    // 表列表
    vector<string> TableList();

    void Finalize();

    void Save();

private:
    map<string, TableDesc::ptr> tables;
};

#endif // _DD_H