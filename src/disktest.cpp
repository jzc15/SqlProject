#include <disk/file.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>

using namespace std;

int main()
{
    system("rm -rf test.data");

    data_t x = alloc_data(PAGE_SIZE);
    data_t y = alloc_data(PAGE_SIZE);
    for(int i = 0; i < PAGE_SIZE; i ++)
    {
        x->data()[i] = rand() & 255;
        y->data()[i] = rand() & 255;
    }

    {
        File file("test.data");
        data_t page1 = file.ReadPage(0);
        memcpy(page1->data(), x->data(), PAGE_SIZE);
        data_t page2 = file.ReadPage(1);
        memcpy(page2->data(), y->data(), PAGE_SIZE);
    }

    {
        File file("test.data");
        data_t page1 = file.ReadPage(0, false);
        assert(memcmp(page1->data(), x->data(), PAGE_SIZE) == 0);
        data_t page2 = file.ReadPage(1, false);
        assert(memcmp(page2->data(), y->data(), PAGE_SIZE) == 0);
    }

    {
        File file("test.data");
        data_t page1 = file.ReadPage(2);
        memcpy(page1->data(), x->data(), PAGE_SIZE);
        data_t page2 = file.ReadPage(3);
        memcpy(page2->data(), y->data(), PAGE_SIZE);
    }

    {
        File file("test.data");
        data_t page1 = file.ReadPage(0, false);
        assert(memcmp(page1->data(), x->data(), PAGE_SIZE) == 0);
        data_t page2 = file.ReadPage(1, false);
        assert(memcmp(page2->data(), y->data(), PAGE_SIZE) == 0);
    }

    {
        File file("test.data");
        data_t page1 = file.ReadPage(2, false);
        assert(memcmp(page1->data(), x->data(), PAGE_SIZE) == 0);
        data_t page2 = file.ReadPage(3, false);
        assert(memcmp(page2->data(), y->data(), PAGE_SIZE) == 0);
    }

    cout << "Test Success" << endl;

    return 0;
}