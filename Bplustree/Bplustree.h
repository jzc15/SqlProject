#ifndef BPLUS_TREE
#define BPLUS_TREE

#include "filesystem/utils/pagedef.h"
#include "ddf/RecordBinary.h"
#include "recordmanager/RecordManager.h"
#include <iostream>
#include <string>

using namespace std;
/*
 *  根节点PageNo = 1，根节点right链接第一个叶节点，left链接最后一个页节点
 *  接口总结
 *  save()
 *      根据PageNo保存树
 *  load()
 *      根据PageNo读取复原树
 *  print()
 *      打印树，用于调试
 *  getRid(ushort pn, ushort sn)
 *      将PageNo和SlotNo封装为rid
 *  getPnSn(uint rid, ushort &pn, ushort &sn)
 *      将rid还原PageNo和SlotNo
 *  compare(RecordBinary &A, RecordBinary &B)
 *      根据type，实现比较
 *  insert(RecordBinary &a, uint rid, bool merge)
 *      插入数据（a, rid)，merge表示是否在上溢，插入时将merge置为0
 *  search(RecordBinary &a)
 *      查询数据a，返回rid，未找到返回不超过它的最大值，如果还不存在则返回最小值
 *  search(RecordBinary &a， ushort &pn, int &z)
 *      查询数据a，返回rid，所在树的位置pn和z，未找到返回不超过它的最大值，如果还不存在则返回最小值
 *  remove(RecordBinary &a)
 *      删除查询数据a，未找到则不做任何操作
 */
class Bplus_tree
{
    uint N = 4;
    uchar type; //0: 可以直接使用字典比较，1：int比较，2：float比较
    
    uint PageNo;
    uint parent;
    uint left;
    uint right;
    string databaseName;
    string tableName;

public:
    bool leaf;
    ushort key_cnt;
    RecordBinary *key;
    uint *son;
    Bplus_tree(string databaseName, string tableName, uchar type, uint PageNo, uint parent);
    void save();
    void load();
    void print();
    uint getRid(ushort pn, ushort sn);
    void getPnSn(uint rid, ushort &pn, ushort &sn);
    bool compare(RecordBinary &A, RecordBinary &B);
    void insert(RecordBinary &a, uint rid, bool merge);
    uint search(RecordBinary &a, ushort &pn, int &z);
    uint search(RecordBinary &a);
    void remove(RecordBinary &a);
    ~Bplus_tree();
};

#endif