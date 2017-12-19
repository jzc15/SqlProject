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
    RecordBinary u = record.Generate();
    if(u.size == 0)
        return false;
    rm.createRecord(tableName, u);
    return true;
}
RecordBinary RecordModel::searchRecord(string tableName, string key, string value, bool head, ushort& PageNo, ushort& SlotNo)
{
    if(head)
        PageNo = SlotNo = 1;
    RecordBinary u;
    TableDescription td(databaseName, tableName);
    Record record(&td);
    while(true)
    {
        u = rm.searchRecord(tableName, PageNo, SlotNo);
        record.Recover(u);
        string s = record.GetString(key);
        if(value.compare(s) == 0)
            return u;
        if(!rm.nextRecord(tableName, PageNo, SlotNo))
            break;
    }
    u.size = 0;
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