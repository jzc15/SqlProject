#ifndef BPLUS_TREE
#define BPLUS_TREE

#include "filesystem/utils/pagedef.h"
#include "ddf/RecordBinary.h"
#include "recordmanager/RecordManager.h"
#include <iostream>
#include <string>

using namespace std;
/*
 * 根节点PageNo = 1
 * 
 */
class Bplus_tree
{
    uint N = 100;
    uchar type; //0: 可以直接使用字典比较，1：int比较，2：float比较
    
    uint PageNo;
    uint parent;
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
    bool compare(RecordBinary &A, RecordBinary &B);
    void insert(RecordBinary &a, uint rid, bool merge);
    //void find(RecordBinary &a, uint& rid);
    ~Bplus_tree();
};

#endif