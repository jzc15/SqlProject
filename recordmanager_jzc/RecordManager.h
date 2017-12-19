#ifndef RECORD_MANAGER
#define RECORD_MANAGER

#include "filesystem/bufmanager/BufPageManager.h"
#include "filesystem/fileio/FileManager.h"
#include "ddf/RecordBinary.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_SHORT_CNT 4095

using namespace std;
/*
 *  接口总结
 *  RecordManger(string databaseName)   
 *      构造一个数据库，创建数据库文件夹
 *  createRecord(string tableName, bitList buf, BufLength length)  
 *      添加数据，如果数据表不存在则创建数据表，返回值表示是否添加成功
 *  searchRecord(string tableName, ushort PageNo, ushort SlotNo, ushort &length)
 *      根据页编号和槽编号查询数据，返回bitList指针，以及数据长度
 *  deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
 *      根据页编号和槽编号查询数据，删除数据
 *  nextRecord(string tableName, ushort& PageNo, ushort& SlotNo)
 *      根据页编号和槽编号查询下一条数据，返回是否存在下一条
 */
class RecordManager
{
    string databaseName;
public:
    RecordManager();
    RecordManager(string databaseName);
    bool createRecord(string tableName, bitList buf, BufLength length);
    bool createRecord(string tableName, RecordBinary rb);
    RecordBinary searchRecord(string tableName, ushort PageNo, ushort SlotNo);
    bool deleteRecord(string tableName, ushort PageNo, ushort SlotNo);
    bool nextRecord(string tableName, ushort& PageNo, ushort& SlotNo);
    bool saveRecord(string tableName, RecordBinary rb,  ushort PageNo, ushort SlotNo);
    ushort getPageCnt(string tableName);
};

#endif