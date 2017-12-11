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
    //btree.print();
}
void del(Bplus_tree& btree, int x)
{
    cout << "---------------delete " << x << endl;
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
    btree.remove(a, 0);
    btree.load();
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
void print()
{
    ///*
    Bplus_tree btree1("testBpulstree", "test", 1, 1, 1);
    btree1.load();
    btree1.print();
    Bplus_tree btree2("testBpulstree", "test", 1, 2, 1);
    btree2.load();
    btree2.print();
    Bplus_tree btree3("testBpulstree", "test", 1, 3, 1);
    btree3.load();
    btree3.print();
    Bplus_tree btree4("testBpulstree", "test", 1, 4, 1);
    btree4.load();
    btree4.print();
    //*/
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

    /*
    Bplus_tree btree2("testBpulstree", "test", 1, 2, 1);
    btree2.load();
    btree2.print();
    Bplus_tree btree3("testBpulstree", "test", 1, 3, 1);
    btree3.load();
    btree3.print();
    Bplus_tree btree4("testBpulstree", "test", 1, 4, 1);
    btree4.load();
    btree4.print();
    */
    insert(btree, 5);

    search(btree, 7);
    search(btree, 12);

    print();
    del(btree, 7);
    del(btree, 12);
    del(btree, 10);
    del(btree, 5);
    del(btree, 9);
    del(btree, 13);
    del(btree, 4);
    del(btree, 6);
    
    print();
    // RecordBinary a, b, c;
    // a.size = 4;
    // a.ptr = new uchar[4];
    // a.ptr[0] = 1;
    // a.ptr[1] = 0;
    // a.ptr[2] = 0;
    // a.ptr[3] = 0;

    // b.size = 4;
    // b.ptr = new uchar[4];
    // b.ptr[0] = 3;
    // b.ptr[1] = 0;
    // b.ptr[2] = 0;
    // b.ptr[3] = 0;
    // btree.insert(b, 2, 0);
    
    
    // c.size = 4;
    // c.ptr = new uchar[4];
    // c.ptr[0] = 2;
    // c.ptr[1] = 0;
    // c.ptr[2] = 0;
    // c.ptr[3] = 0;
    
    // btree.insert(c, 3, 0);

    // btree.print();
    
    // Bplus_tree btree2("testBpulstree", "test", 3, 1, 0);
    // btree2.print();
    // btree2.load();
    // btree2.print();

    
    return 0;
}