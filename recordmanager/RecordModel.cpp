#include "RecordModel.h"

RecordModel::RecordModel()
{
    databaseName = "";
}
RecordModel::RecordModel(string databaseName)
{
    this -> databaseName = databaseName;
    rm = (RecordManager)(databaseName);
    mkdir(databaseName.c_str() ,0755);
}
bool RecordModel::createFile(string filename)
{
    filename = databaseName + "/" + filename;
    fm.createFile(filename.c_str());
    return true;
}
bool RecordModel::deleteFile(string filename)
{
    filename = databaseName + "/" + filename;
    int fileID;
    //cout << "fileID = " << fileID << endl;
    //cout << "filename = " << filename << endl;
    fm.openFile(filename.c_str(), fileID);
    fm.closeFile(fileID); 
    if(remove(filename.c_str()) == 0)
        return true;
    return false;
}
int RecordModel::openFile(string filename)
{
    filename = databaseName + "/" + filename;
    int fileID;
    fm.openFile(filename.c_str(), fileID);
    return fileID;
}
bool RecordModel::closeFile(int fileID)
{
    return fm.closeFile(fileID);
}
bool RecordModel::closeFile(string filename)
{
    filename = databaseName + "/" + filename;
    int fileID;
    fm.openFile(filename.c_str(), fileID);
    return fm.closeFile(fileID);
}
bool RecordModel::insertRecord(string tableName, Record record)
{
    bitList u;
    ushort length = -1;
    // u = getushortList(databaseName, tableName, record, length);
    if(length == -1)
        return false;
    rm.createRecord(tableName, u, length);
    return true;
}
RecordBinary RecordModel::searchRecord(string tableName, string key, string, bool head, ushort& PageNo, ushort& SlotNo)
{
    if(head)
        PageNo = SlotNo = 1;
    RecordBinary u;
    while(true)
    {
        u = rm.searchRecord(tableName, PageNo, SlotNo);
        //if(check(databaseName, tableName, u, length, key, value)) break;
        if(!rm.nextRecord(tableName, PageNo, SlotNo))
            return u;
    }
    //return toJson(databaseName, tableName， u, length);
    return u;
}
bool RecordModel::deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
{
    return rm.deleteRecord(tableName, PageNo, SlotNo);
}
bool RecordModel::updateRecord(string tableName, Record record, ushort PageNo, ushort SlotNo)
{
    if(!deleteRecord(tableName, PageNo, SlotNo))
        return false;
    return insertRecord(tableName, record);
}