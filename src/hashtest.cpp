#include <indices/hashtable.h>
#include <indices/multihashtable.h>
#include <iostream>
#include <cassert>

using namespace std;

int main()
{
    string filename = "debug.hash";

    system(("rm " + filename).c_str());
    {
        cout << "[1]" << endl;
        HashTable::ptr hash = make_shared<HashTable>(filename, 4);
        for(int i = 0; i < 10000; i ++)
        {
            hash->Insert(int_data(i), i);
        }
        for(int i = 1; i < 10000; i += i)
        {
            hash->Delete(int_data(i));
        }
        hash->Delete(int_data(200));
        hash->Insert(int_data(200), 3);
    }
    {
        cout << "[2]" << endl;
        HashTable::ptr hash = make_shared<HashTable>(filename, 4);
        assert(hash->Exists(int_data(0)));
        assert(!hash->Exists(int_data(1)));
        assert(hash->Exists(int_data(3)));

        assert(hash->Fetch(int_data(0)) == 0);
        assert(hash->Fetch(int_data(200)) == 3);
        assert(hash->Fetch(int_data(99)) == 99);
    }

    filename = "debug2.hash";
    system(("rm " + filename).c_str());
    {
        cout << "[3]" << endl;
        MultiHashTable::ptr hash = make_shared<MultiHashTable>(filename, 4);
        for(int i = 0; i < 1000; i ++)
        {
            cout << i << endl;
            for(int j = 0; j < 10000; j ++)
            {
                hash->Insert(int_data(i), j);
            }
        }
        cout << "111111" << endl;
        for(int i = 100; i < 200; i ++)
        {
            for(int j = 1000; j < 2000; j ++)
            {
                hash->Delete(int_data(i), j);
            }
        }
        cout << "22222" << endl;
        for(int i = 0; i < 10; i ++)
        {
            for(int j = 0; j < 10000; j ++)
            {
                hash->Delete(int_data(i), j);
            }
        }
        cout << "333333" << endl;
    }
    {
        cout << "[4]" << endl;
        MultiHashTable::ptr hash = make_shared<MultiHashTable>(filename, 4);
        for(int i = 0; i < 10; i ++)
        {
            assert(hash->Count(int_data(i)) == 0);
        }
        for(int i = 100; i < 200; i ++)
        {
            assert(hash->Count(int_data(i)) == 10000 - (2000 - 1000));
        }
        for(int i = 300; i < 1000; i ++)
        {
            vector_t data = hash->Fetch(int_data(i));
            assert(data->size() == 10000);
            for(int j = 0; j < 10000; j ++)
            {
                assert(data->at(j) == j);
            }
        }
    }

    cout << "Test Success" << endl;

    return 0;
}