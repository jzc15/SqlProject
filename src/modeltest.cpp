#include <datamanager/slotsfile.h>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <cstring>

using namespace std;

int main()
{
    system("rm -rf test.data");
    data_t datum[10];
    datum[0] = alloc_data(10);
    datum[1] = alloc_data(10);
    datum[2] = alloc_data(20);
    datum[3] = alloc_data(1024);
    datum[4] = alloc_data(4096);
    datum[5] = alloc_data(4096);
    datum[6] = alloc_data(4096);
    datum[7] = alloc_data(10);
    datum[8] = alloc_data(20);
    datum[9] = alloc_data(20);
    for(int i = 0; i < 10; i ++)
    {
        for(int j = 0; j < datum[i]->size(); j ++)
        {
            datum[i]->data()[j] = rand() & 255;
        }
    }

    { // create
        SlotsFile file("test.data");
        assert(file.Insert(datum[0]) == 0);
        assert(file.Insert(datum[1]) == 1);
        for(int i = 2; i <= 9; i ++)
        {
            file.Insert(datum[i]);
        }
        file.Delete(1);
    }

    { // load
        SlotsFile file("test.data");

        data_t d0 = file.Fetch(0);
        data_t d1 = file.Fetch(1);
        data_t d2 = file.Fetch(2);

        assert(d0->size() == datum[0]->size());
        assert(d1 == nullptr);
        assert(d2->size() == datum[2]->size());

        assert(memcmp(d0->data(), datum[0]->data(), d0->size()) == 0);
        assert(memcmp(d2->data(), datum[2]->data(), d2->size()) == 0);
    }

    { // load
        SlotsFile file("test.data");

        file.Begin();

        for(int i = 2; i <= 4; i ++)
        {
            cout << "Running " << i << endl;
            data_t data = file.Next();
            assert(data != nullptr);
            assert(data->size() == datum[i]->size());
            assert(memcmp(data->data(), datum[i]->data(), data->size()) == 0);
        }
        for(int i = 7; i <= 9; i ++)
        {
            cout << "Running " << i << endl;
            data_t data = file.Next();
            assert(data != nullptr);
            assert(data->size() == datum[i]->size());
            assert(memcmp(data->data(), datum[i]->data(), data->size()) == 0);
        }
        data_t data = file.Next();
        assert(memcmp(data->data(), datum[5]->data(), data->size()) == 0);
        data = file.Next();
        assert(memcmp(data->data(), datum[6]->data(), data->size()) == 0);
    }

    cout << "Test Success" << endl;
    
    return 0;
}