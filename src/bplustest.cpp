#include <disk/common.h>
#include <indices/bplustree.h>
#include <iostream>
#include <cassert>

using namespace std;

int main()
{
    if (exists("debug.index")) rmfile("debug.index");

    {
        BPlusTree::ptr tree = make_shared<BPlusTree>("debug.index", INT_ENUM);

        tree->Insert(int_data(3), 4);
        tree->Insert(int_data(3), 4);
        tree->Insert(int_data(3), 5);
        assert(tree->IsKeyExists(int_data(3)));
        assert(tree->LTCount(int_data(3)) == 0);
        assert(tree->EQCount(int_data(3)) == 3);

        tree->Insert(int_data(2), 4);
        assert(tree->LTCount(int_data(3)) == 1);
        assert(tree->EQCount(int_data(3)) == 3);

        tree->Insert(int_data(19), 4);
        assert(tree->IsKeyExists(int_data(3)));
        assert(tree->IsKeyExists(int_data(2)));
        assert(!tree->IsKeyExists(int_data(1)));
        assert(tree->LTCount(int_data(3)) == 1);
        assert(tree->EQCount(int_data(3)) == 3);
        assert(tree->TotalCount() == 5);

        tree->Insert(int_data(27), 6);
        tree->Insert(int_data(28), 6);
        tree->Insert(int_data(27), 7);
        tree->Insert(int_data(29), 6);
        tree->Insert(int_data(37), 6);
        tree->Insert(int_data(17), 6);
        assert(tree->IsKeyExists(int_data(27)));
        assert(!tree->IsKeyExists(int_data(26)));
        assert(tree->EQCount(int_data(28)) == 1);
        assert(tree->EQCount(int_data(27)) == 2);
        assert(tree->LECount(int_data(28)) == 9);
    }

    {
        BPlusTree::ptr tree = make_shared<BPlusTree>("debug.index", INT_ENUM);

        auto iter = tree->Begin();
        assert(equals(iter.Key(), int_data(2)));
        assert(iter.Value() == 4);
        iter.Next();
        assert(!iter.End());
        assert(equals(iter.Key(), int_data(3)));
        assert(iter.Value() == 4);
        iter.Next();
        assert(equals(iter.Key(), int_data(3)));
        assert(iter.Value() == 4);
        iter.Next();
        assert(equals(iter.Key(), int_data(3)));
        assert(iter.Value() == 5);
        iter.Next();
        assert(equals(iter.Key(), int_data(17)));
        assert(iter.Value() == 6);
        iter.Next();
        assert(!iter.End());
    }

    {
        BPlusTree::ptr tree = make_shared<BPlusTree>("debug.index", INT_ENUM);
        
        assert(tree->EQCount(int_data(3)) == 3);
        tree->Delete(int_data(3), 4);
        assert(tree->EQCount(int_data(3)) == 2);
        tree->Delete(int_data(3), 4);
        assert(tree->EQCount(int_data(3)) == 1);
        tree->Delete(int_data(3), 5);
        assert(!tree->IsKeyExists(int_data(3)));
        
        tree->Delete(int_data(2), 4);
        tree->Delete(int_data(17), 6);
        tree->Delete(int_data(19), 4);
        tree->Delete(int_data(27), 6);
        tree->Delete(int_data(27), 7);
        tree->Delete(int_data(37), 6);

        assert(tree->TotalCount() == 2);
        assert(tree->IsKeyExists(int_data(28)));
        assert(tree->LTCount(int_data(29)) == 1);
        assert(tree->EQCount(int_data(28)) == 1);
    }

    cout << "Test Success" << endl;

    return 0;
}
