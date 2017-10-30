# SqlProject

## 20171021 jzc

### RecordManager.h

####  接口总结
 *  RecordManger(string databaseName)   
       
        构造一个数据库，创建数据库文件夹

 *  createRecord(string tableName, bitList buf, BufLength length)  
       
        添加数据，如果数据表不存在则创建数据表，返回值表示是否添加成功

 *  searchRecord(string tableName, ushort PageNo, ushort SlotNo, ushort &length)
       
        根据页编号和槽编号查询数据，返回ushortList指针，以及数据长度

 *  deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
       
        根据页编号和槽编号查询数据，删除数据

 *  nextRecord(string tableName, ushort& PageNo, ushort& SlotNo)
       
        根据页编号和槽编号查询下一条数据，返回是否存在下一条
 

### RecordModel.h


####  接口总结
 *  RecordModel(string databaseName)   
       
        构造一个数据库，创建数据库文件夹

 *  createFile(string filename)
       
        创建文件,文件名不需要带目录

 *  deleteFile(string filename)
       
        删除文件并返回是否成功

 *  openFile(string filename)
       
        打开文件返回fileID

 *  closeFile(int fileID)
    
        根据fileID关闭文件

 *  closeFile(string filename)
       
        根据文件名关闭文件

 *  insertRecord(string tableName, Record record)
       
        根据表名和json记录插入数据

 *  searchRecord(string tableName, string key, string, bool head, ushort& PageNo, ushort& SlotNo)
        
        根据表名和key,value查询一条数据,head表示是否从头开始查找，如果不是那么需要给定PageNo和SlotNo;
        从（PageNo，SlotNo）开始查找，作为第一个check的项;
        返回Record，数据堆RID （PageNo，SlotNo）

 *  deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
        
        根据PageNo和SlotNo删除指定数据

 *  updateRecord(string tableName, Record record， ushort PageNo, ushort SlotNo)
        
        根据PageNo和SlotNo，将此数据更新为record
