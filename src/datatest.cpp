#include <datamanager/slotsfile.h>
#include <datamanager/vectorfile.h>
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
        assert(file.Insert(datum[0]) == RID(1, 0));
        assert(file.Insert(datum[1]) == RID(1, 1));
        for(int i = 2; i <= 9; i ++)
        {
            file.Insert(datum[i]);
        }
        file.Delete(RID(1, 1));
    }

    { // load
        SlotsFile file("test.data");

        data_t d0 = file.Fetch(RID(1, 0));
        data_t d1 = file.Fetch(RID(1, 1));
        data_t d2 = file.Fetch(RID(1, 2));

        assert(d0->size() == datum[0]->size());
        assert(d1 == nullptr);
        assert(d2->size() == datum[2]->size());

        assert(memcmp(d0->data(), datum[0]->data(), d0->size()) == 0);
        assert(memcmp(d2->data(), datum[2]->data(), d2->size()) == 0);
    }

    { // load
        SlotsFile file("test.data");

        file.Begin();

        for(int i = 2; i <= 9; i ++)
        {
            cout << "Running " << i << endl;
            data_t data = file.Next();
            assert(data != nullptr);
            assert(data->size() == datum[i]->size());
            assert(memcmp(data->data(), datum[i]->data(), data->size()) == 0);
        }
    }

    system("rm test2.data");
    const int N = 55073;
    int vecs1[N], vecs2[N];
    int bigvecs[10];
    {
        VectorFile::ptr file = make_shared<VectorFile>("test2.data");
        for(int i = 0; i < N; i ++)
            vecs1[i] = file->NewVector(i);
        for(int i = 0; i < 10; i ++)
        {
            bigvecs[i] = file->NewVector(0);
            vector_t data = file->Fetch(bigvecs[i]);
            data->resize(N+i);
            for(int j = 0; j < N + i; j ++)
            {
                data->data()[j] = j;
            }
            bigvecs[i] = file->Save(bigvecs[i], data);
        }
        for(int i = 0; i < N; i ++)
            vecs2[i] = file->NewVector(i);
    }
    {
        VectorFile::ptr file = make_shared<VectorFile>("test2.data");
        for(int i = 0; i < N; i ++)
        {
            vector_t data = file->Fetch(vecs1[i]);
            assert(data->size() == 1);
            assert(data->at(0) == i);
        }
        for(int i = 0; i < 10; i ++)
        {
            vector_t data = file->Fetch(bigvecs[i]);
            assert(data->size() == N+i);
            for(int j = 0; j < N + i; j ++)
            {
                assert(data->at(j) == j);
            }
        }
        for(int i = 0; i < N; i ++)
        {
            vector_t data = file->Fetch(vecs2[i]);
            assert(data->size() == 1);
            assert(data->at(0) == i);
        }
    }

    cout << "Test Success" << endl;
    
    return 0;
}