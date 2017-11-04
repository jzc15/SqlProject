#include<iostream>
#include "filesystem/utils/pagedef.h"
#include "ddf/RecordBinary.h"
#include "recordmanager/RecordManager.h"
#include "Bplustree/Bplustree.h"

using namespace std;
int main()
{
    Bplus_tree btree("test", "test", 3, 1, 1);
    
    RecordBinary a;
    a.size = 4;
    a.ptr = new uchar[4];
    a.ptr[0] = 0;
    a.ptr[1] = 1;
    a.ptr[2] = 3;
    a.ptr[3] = 4;
    
    btree.insert(a, 1, 0);
    a.ptr[3] = 5;
    btree.insert(a, 2, 0);
    a.ptr[2] = 6;
    btree.insert(a, 3, 0);

    btree.print();
    return 0;
}