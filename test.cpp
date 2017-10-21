#include<iostream>
#include "RecordManager.h"

using namespace std;
int main()
{
    RecordManager *rm = new RecordManager("testdir");
    ushort u[4];
    u[0] = 1;
    u[1] = 2;
    u[2] = 3;
    u[3] = 4;
    rm -> createRecord("test", u, 4);
    u[3] = 5;
    rm -> createRecord("test", u, 4);
    ushort u0[3];
    u0[0] = 6;
    u0[1] = 7;
    rm -> createRecord("test", u0, 3);

    u0[2] = 8;
    ushortList r;
    ushort l;
    r = rm -> searchRecord("test", 1, 1, l);
    cout << "r[3] = " << r[3] << endl;
    r = rm -> searchRecord("test", 1, 2, l);
    cout << "r[3] = " << r[3] << endl;
    r = rm -> searchRecord("test", 1, 3, l);
    cout << "r[0] = " << r[0] << endl;
    return 0;
}