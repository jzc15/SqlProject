#ifndef RECORD_MODEL
#define RECORD_MODEL

#include<iostream>
#include <sys/stat.h>
#include "RecordManager.h"
#include "common/Record.h"
using namespace std;
/*
 *  接口总结
 *  RecordModel(string databaseName)   
 *      构造一个数据库，创建数据库文件夹
 *  createFile(string filename)
 *      创建文件,文件名不需要带目录
 *  deleteFile(string filename)
 *      删除文件并返回是否成功
 *  openFile(string filename)
 *      打开文件返回fileID
 *  closeFile(int fileID)
 *      根据fileID关闭文件
 *  closeFile(string filename)
 *      根据文件名关闭文件
 *  insertRecord(string tableName, Record record)
 *      根据表名和json记录插入数据
 *  searchRecord(string tableName, string key, string, bool head, ushort& PageNo, ushort& SlotNo)
 *      根据表名和key,value查询一条数据,head表示是否从头开始查找，如果不是，那么需要给定PageNo和SlotNo;
 *      从（PageNo，SlotNo）开始查找，作为第一个check的项;
 *      返回json数据，数据堆RID （PageNo，SlotNo）
 *  deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
 *      根据PageNo和SlotNo删除指定数据
 *  updateRecord(string tableName, Record record， ushort PageNo, ushort SlotNo)
 *      根据PageNo和SlotNo，将此数据更新为record
 * 
 *  待实现接口
 *  getushortList(databaseName, tableName, record, length); 
 *      根据数据库名，数据表名，记录字符串，返回unsigned short 数组及其长度
 *  check(databaseName, tableName, u, length, key, value)
 *      根据数据库名，数据表名，unsigned short 数组， key, value, 检查是否符合
 *  toJson(u, length);
 *      根据数据库名，数据表名，unsigned short 数组， 返回json字符串
 */
class RecordModel
{
    FileManager fm;
    RecordManager rm;
    string databaseName;
public:
    RecordModel();
    RecordModel(string databaseName);
    bool createFile(string filename);
    bool deleteFile(string filename);
    int openFile(string filename);
    bool closeFile(int fileID);
    bool closeFile(string filename);
    bool insertRecord(string tableName, Record record);
    bitList searchRecord(string tableName, string key, string, bool head, ushort& PageNo, ushort& SlotNo);
    bool deleteRecord(string tableName, ushort PageNo, ushort SlotNo);
    bool updateRecord(string tableName, Record record, ushort PageNo, ushort SlotNo);
};

#endif