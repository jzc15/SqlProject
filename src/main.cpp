#include <iostream>
#include <frontend/helper.h>

using namespace std;

int main(int argc, char* argv[])
{
    Program* p = parseFile("../test.sql");
    // Program* p = parseFile(stdin);

    p->run();

    delete p;

    return 0;
}