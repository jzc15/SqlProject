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