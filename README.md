# SqlProject

## 20171021 jzc

### 模块说明

* `common` : 公工模块，定义配置/类型信息

* `ddf` : 数据库/数据表描诉文件，实现了数据库/表/记录的描述类，实现了记录序列化/反序列化等接口，已测试

* `filesystem` : SB页式文件系统（头文件定义变量，贼傻逼）

* `frontend` : 前段模块，负责解析命令

* `json11` : 外部引用json模块

* `recordmanager` : 在页式文件系统之上的记录管理系统

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

#### 单元测试
```c++
#include<iostream>
#include "RecordManager.h"
#include "RecordModel.h"

using namespace std;
int main()
{
    
    RecordModel rml("testdir");
    cout << rml.deleteFile("test") << endl;
    RecordManager *rm = new RecordManager("testdir");
    
    uchar u[4];
    u[0] = 'a';
    u[1] = 'b';
    u[2] = 'c';   
    u[3] = 'd';
    rm -> createRecord("test", &u[0], 4);
    u[2] = 'e';
    rm -> createRecord("test", &u[0], 4);
    uchar u0[3];
    u0[0] = 't';
    u0[1] = 'e';
    u0[2] = 't';
    rm -> createRecord("test", &u0[0], 3);

    u0[2] = 8;
    RecordBinary r;
    r = rm -> searchRecord("test", 1, 1);
    cout << "r[3] = " << r.ptr[3] << endl;
    r = rm -> searchRecord("test", 1, 2);
    cout << "r[2] = " << r.ptr[2] << endl;
    rm -> deleteRecord("test", 1, 2);
    r = rm -> searchRecord("test", 1, 2);
    cout << "r[0] = " << r.ptr[0] << endl;

    
    rml.createFile("testmodel");
    rml.deleteFile("testmodel");
    return 0;
}
```

结果：
```
r[3] = d
r[2] = e
r[0] = t

```

### BplusTree.h

 *  根节点PageNo = 1，根节点right链接第一个叶节点，left链接最后一个页节点

 ####  接口总结
 
 *  save()
 
        根据PageNo保存树

 *  load()
       
        根据PageNo读取复原树

 *  print()
       
        打印树，用于调试

 *  getRid(ushort pn, ushort sn)
       
        将PageNo和SlotNo封装为rid

 *  getPnSn(uint rid, ushort &pn, ushort &sn)
       
        将rid还原PageNo和SlotNo

 *  compare(RecordBinary &A, RecordBinary &B)
       
        根据type，实现比较

 *  insert(RecordBinary &a, uint rid, bool merge)
       
        插入数据（a, rid)，merge表示是否在上溢，插入时将merge置为0

 *  search(RecordBinary &a)
       
        查询数据a，返回rid，未找到返回不超过它的最大值，如果还不存在则返回最小值

 *  search(RecordBinary &a， ushort &pn, int &z)
       
        查询数据a，返回rid，所在树的位置pn和z，未找到返回不超过它的最大值，如果还不存在则返回最小值

 *  remove(RecordBinary &a)
       
        删除查询数据a，未找到则不做任何操作

#### 单元测试 N=4
```c++
#include<iostream>
#include "filesystem/utils/pagedef.h"
#include "ddf/RecordBinary.h"
#include "recordmanager/RecordManager.h"
#include "recordmanager/RecordModel.h"
#include "Bplustree/Bplustree.h"

using namespace std;
void insert(Bplus_tree& btree, int x)
{
    cout << "---------------insert " << x << endl;
    RecordBinary a;
    uint rid = x*x;
    a.size = 4;
    a.ptr = new uchar[4];
    a.ptr[0] = x%256;
    x /= 256;
    a.ptr[1] = x%256;
    x /= 256;
    a.ptr[2] = x%256;
    x /= 256;
    a.ptr[3] = x;
    btree.insert(a, rid, 0);
    btree.load();
    btree.print();
}
void search(Bplus_tree& btree, int x)
{

    cout << "---------------search " << x << endl;
    RecordBinary a;
    uint rid = x*x;
    a.size = 4;
    a.ptr = new uchar[4];
    a.ptr[0] = x%256;
    x /= 256;
    a.ptr[1] = x%256;
    x /= 256;
    a.ptr[2] = x%256;
    x /= 256;
    a.ptr[3] = x;

    cout << "accept " << rid << ", get " << btree.search(a) << endl;
}
int main()
{
    
    RecordModel rml("testBpulstree");
    cout << rml.deleteFile("test") << endl;
    Bplus_tree btree("testBpulstree", "test", 1, 1, 1);
    btree.save();
    insert(btree, 10);
    insert(btree, 13);
    insert(btree, 12);
    insert(btree, 4);
    insert(btree, 7);
    insert(btree, 6);
    insert(btree, 9);
    insert(btree, 8);

    Bplus_tree btree2("testBpulstree", "test", 1, 4, 1);
    btree2.load();
    btree2.print();

    insert(btree, 5);

    search(btree, 7);
    search(btree, 12);

    
    return 0;
}
```
结果
```
---------------insert 10
-----------bplusTree-------------
type = 1
leaf = 1
PageNo = 1
key_cnt = 1
left = 1
right = 1
parent = 1
key[0].size = 4
key[0].ptr[0] = 10
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
son[0] = 100
-----------bplusTree----end------
---------------insert 13
-----------bplusTree-------------
type = 1
leaf = 1
PageNo = 1
key_cnt = 2
left = 1
right = 1
parent = 1
key[0].size = 4
key[0].ptr[0] = 10
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 13
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
son[0] = 100
son[1] = 169
-----------bplusTree----end------
---------------insert 12
-----------bplusTree-------------
type = 1
leaf = 1
PageNo = 1
key_cnt = 3
left = 1
right = 1
parent = 1
key[0].size = 4
key[0].ptr[0] = 10
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 12
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
key[2].size = 4
key[2].ptr[0] = 13
key[2].ptr[1] = 0
key[2].ptr[2] = 0
key[2].ptr[3] = 0
son[0] = 100
son[1] = 144
son[2] = 169
-----------bplusTree----end------
---------------insert 4
-----------bplusTree-------------
type = 1
leaf = 1
PageNo = 1
key_cnt = 4
left = 1
right = 1
parent = 1
key[0].size = 4
key[0].ptr[0] = 4
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 10
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
key[2].size = 4
key[2].ptr[0] = 12
key[2].ptr[1] = 0
key[2].ptr[2] = 0
key[2].ptr[3] = 0
key[3].size = 4
key[3].ptr[0] = 13
key[3].ptr[1] = 0
key[3].ptr[2] = 0
key[3].ptr[3] = 0
son[0] = 16
son[1] = 100
son[2] = 144
son[3] = 169
-----------bplusTree----end------
---------------insert 7
-----------bplusTree-------------
type = 1
leaf = 0
PageNo = 1
key_cnt = 2
left = 2
right = 3
parent = 1
key[0].size = 4
key[0].ptr[0] = 4
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 12
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
son[0] = 3
son[1] = 2
-----------bplusTree----end------
---------------insert 6
-----------bplusTree-------------
type = 1
leaf = 0
PageNo = 1
key_cnt = 2
left = 2
right = 3
parent = 1
key[0].size = 4
key[0].ptr[0] = 4
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 12
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
son[0] = 3
son[1] = 2
-----------bplusTree----end------
---------------insert 9
totalPage = 3
-----------bplusTree-------------
type = 1
leaf = 0
PageNo = 1
key_cnt = 3
left = 2
right = 3
parent = 1
key[0].size = 4
key[0].ptr[0] = 4
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 7
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
key[2].size = 4
key[2].ptr[0] = 12
key[2].ptr[1] = 0
key[2].ptr[2] = 0
key[2].ptr[3] = 0
son[0] = 3
son[1] = 4
son[2] = 2
-----------bplusTree----end------
---------------insert 8
-----------bplusTree-------------
type = 1
leaf = 0
PageNo = 1
key_cnt = 3
left = 2
right = 3
parent = 1
key[0].size = 4
key[0].ptr[0] = 4
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 7
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
key[2].size = 4
key[2].ptr[0] = 12
key[2].ptr[1] = 0
key[2].ptr[2] = 0
key[2].ptr[3] = 0
son[0] = 3
son[1] = 4
son[2] = 2
-----------bplusTree----end------
-----------bplusTree-------------
type = 1
leaf = 1
PageNo = 4
key_cnt = 4
left = 3
right = 2
parent = 1
key[0].size = 4
key[0].ptr[0] = 7
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 8
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
key[2].size = 4
key[2].ptr[0] = 9
key[2].ptr[1] = 0
key[2].ptr[2] = 0
key[2].ptr[3] = 0
key[3].size = 4
key[3].ptr[0] = 10
key[3].ptr[1] = 0
key[3].ptr[2] = 0
key[3].ptr[3] = 0
son[0] = 49
son[1] = 64
son[2] = 81
son[3] = 100
-----------bplusTree----end------
---------------insert 5
-----------bplusTree-------------
type = 1
leaf = 0
PageNo = 1
key_cnt = 3
left = 2
right = 3
parent = 1
key[0].size = 4
key[0].ptr[0] = 4
key[0].ptr[1] = 0
key[0].ptr[2] = 0
key[0].ptr[3] = 0
key[1].size = 4
key[1].ptr[0] = 7
key[1].ptr[1] = 0
key[1].ptr[2] = 0
key[1].ptr[3] = 0
key[2].size = 4
key[2].ptr[0] = 12
key[2].ptr[1] = 0
key[2].ptr[2] = 0
key[2].ptr[3] = 0
son[0] = 3
son[1] = 4
son[2] = 2
-----------bplusTree----end------
---------------search 7
accept 49, get 49
---------------search 12
accept 144, get 144

```