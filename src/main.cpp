#include<iostream>
#include "RecordManager.h"
#include "RecordModel.h"
using namespace std;
int main()
{
    
    RecordModel rml;
    rml.deleteFile("testdir/test");
    RecordManager *rm = new RecordManager("testdir");
    
    char u[4];
    u[0] = 'a';
    u[1] = 'b';
    u[2] = 'c';   
    u[3] = 'd';
    rm -> createRecord("test", &u[0], 4);
    u[3] = 'e';
    rm -> createRecord("test", &u[0], 4);
    char u0[3];
    u0[0] = 't';
    u0[1] = 'e';
    rm -> createRecord("test", &u0[0], 3);

    u0[2] = 8;
    char* r;
    ushort l;
    r = rm -> searchRecord("test", 1, 1, l);
    cout << "r[3] = " << r[3] << endl;
    r = rm -> searchRecord("test", 1, 2, l);
    cout << "r[3] = " << r[3] << endl;
    rm -> deleteRecord("test", 1, 2);
    r = rm -> searchRecord("test", 1, 2, l);
    cout << "r[0] = " << r[0] << endl;

   
    rml.createFile("testmodel");
    rml.deleteFile("testmodel");
    return 0;
}