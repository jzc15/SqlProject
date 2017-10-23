#include <iostream>
#include <frontend/helper.h>

using namespace std;

int main(int argc, char* argv[])
{
    parseFile("../test.sql");
    // parseFile(stdin);
    return 0;
}