#include "bplustree.h"
#include <cassert>
#include <iostream>
#include <cstring>

using namespace std;

BPlusTree::Iterator::Iterator(BPlusTree* tree, node_t* node, int key_pos, int value_pos)
    : tree(tree), node(node), key_pos(key_pos), value_pos(value_pos)
{
}

data_t BPlusTree::Iterator::Key()
{
    assert(0 <= key_pos && key_pos < node->size);
    return tree->key_file->Fetch(node->key_rids[key_pos]);
}
int BPlusTree::Iterator::Value()
{
    assert(0 <= key_pos && key_pos < node->size);
    vector_t value_list = tree->data_file->Fetch(node->children_entries[key_pos]);
    assert(0 <= value_pos && value_pos < node->values_count[key_pos]);
    return value_list->data()[value_pos];
}

void BPlusTree::Iterator::Prev()
{
    if (End()) return;
    assert(0 <= key_pos && key_pos < node->size);

    if (value_pos != 0)
    {
        value_pos --;
    } else if (key_pos != 0) {
        key_pos --;
        value_pos = node->values_count[key_pos]-1;
    } else {
        int page_id = node->prev_page_id;
        node = NULL;
        key_pos = -1;
        value_pos = -1;
        if (page_id != -1)
        {
            node = tree->LoadNode(page_id);
            key_pos = node->size - 1;
            value_pos = node->values_count[key_pos]-1;
        }
    }
}
void BPlusTree::Iterator::Next()
{
    if (End()) return;
    assert(0 <= key_pos && key_pos < node->size);
    assert(0 <= value_pos && value_pos < node->values_count[key_pos]);

    if (value_pos+1 < node->values_count[key_pos])
    {
        value_pos ++;
    } else if (key_pos+1 < node->size) {
        key_pos ++;
        value_pos = 0;
    } else {
        int page_id = node->next_page_id;
        node = NULL;
        key_pos = -1;
        value_pos = -1;
        if (page_id != -1)
        {
            node = tree->LoadNode(page_id);
            key_pos = 0;
            value_pos = 0;
        }
    }
}
bool BPlusTree::Iterator::End()
{
    return node == NULL;
}

BPlusTree::BPlusTree(const string& filename, type_t type)
    : filename(filename), type(type), type_fixed(is_type_fixed(type_name(type)))
{
    assert(sizeof(header_t) < PAGE_SIZE);
    assert(sizeof(node_t) < PAGE_SIZE);
    assert(N%2 == 0);

    this->b_file = make_shared<File>(filename);
    this->key_file = make_shared<SlotsFile>(filename + ".key");
    this->data_file = make_shared<VectorFile>(filename + ".data");

    Init();
}

BPlusTree::~BPlusTree()
{

}

void BPlusTree::RemoveIndex(const string& filename)
{
    if (exists(filename)) rmfile(filename);
    if (exists(filename + ".key")) rmfile(filename + ".key");
    if (exists(filename + ".data")) rmfile(filename + ".data");
}

void BPlusTree::Debug()
{
    cout << "=============DEBUG==============" << endl;
    Debug(LoadNode(header->root_page_id));
}

void BPlusTree::Insert(data_t key, int value)
{
    node_t* root = LoadNode(header->root_page_id);
    if (root->size == N)
    {
        node_t* s = NewNode();
        header->root_page_id = s->page_id;
        s->is_leaf = false;
        s->size = 1;
        data_t max_key = key_file->Fetch(root->key_rids[root->size-1]);
        s->key_rids[0] = key_file->Insert(max_key);
        s->children_entries[0] = root->page_id;
        s->values_count[0] = header->total_count;
        InsertNotFull(s, key, value);
    } else {
        InsertNotFull(root, key, value);
    }
    header->total_count ++;
}

void BPlusTree::Delete(data_t key, int value)
{
    node_t* root = LoadNode(header->root_page_id);
    if (root->size == 2 && root->values_count[0] == N/2 && root->values_count[1] == N/2)
    {
        // FIXME 回收root
        MergeChildren(root, 0);
        node_t* s = LoadNode(root->children_entries[0]);
        header->root_page_id = s->page_id;
        DeleteLargeEnough(s, key, value);
    } else {
        DeleteLargeEnough(root, key, value);
    }
    header->total_count --;
}

bool BPlusTree::IsKeyExists(data_t key)
{
    search_rst rst = SearchGE(LoadNode(header->root_page_id), key);
    return rst.eq_count != 0;
}

int BPlusTree::TotalCount()
{
    return header->total_count;
}

int BPlusTree::LTCount(data_t key)
{
    search_rst rst = SearchGE(LoadNode(header->root_page_id), key);
    return rst.lt_count;
}

int BPlusTree::EQCount(data_t key)
{
    search_rst rst = SearchGE(LoadNode(header->root_page_id), key);
    return rst.eq_count;
}

int BPlusTree::LECount(data_t key)
{
    search_rst rst = SearchGE(LoadNode(header->root_page_id), key);
    return rst.lt_count + rst.eq_count;
}

BPlusTree::Iterator BPlusTree::Begin()
{
    int page_id = header->first_page_id;
    node_t* node = LoadNode(page_id);
    if (node->size == 0) return Iterator(this, NULL, -1, -1);
    return Iterator(this, node, 0, 0);
}

BPlusTree::Iterator BPlusTree::Lower(data_t key)
{
    search_rst rst = SearchGE(LoadNode(header->root_page_id), key);
    return Iterator(this, rst.node, rst.pos, 0);
}

BPlusTree::Iterator BPlusTree::Upper(data_t key)
{
    search_rst rst = SearchGE(LoadNode(header->root_page_id), key);
    auto iter = Iterator(this, rst.node, rst.pos, rst.eq_count-1);
    iter.Next();
    return iter;
}

void BPlusTree::Init()
{
    header = (header_t*)b_file->ReadPage(0)->data();
    if (header->page_type != PAGE_HEADER)
    {
        header->page_type = PAGE_HEADER;
        node_t* root = NewNode();
        header->first_page_id = header->last_page_id = header->root_page_id = root->page_id;
        header->total_count = 0;
        root->is_leaf = true;
    }
}

BPlusTree::node_t* BPlusTree::NewNode()
{
    node_t* node = (node_t*)b_file->NewPage()->data();
    node->page_type = PAGE_NODE;
    node->page_id = b_file->CurrentPage();
    node->is_leaf = false;
    node->next_page_id = -1;
    node->prev_page_id = -1;
    node->size = 0;
    memset(node->key_rids, -1, sizeof(node->key_rids));
    memset(node->children_entries, -1, sizeof(node->children_entries));
    memset(node->values_count, 0, sizeof(node->values_count));

    return node;
}

BPlusTree::node_t* BPlusTree::LoadNode(int page)
{
    return (node_t*)b_file->ReadPage(page)->data();
}

void BPlusTree::Output(data_t data)
{
    switch(type)
    {
    case INT_ENUM:
        cout << *(int*)(data->data());
        break;
    case FLOAT_ENUM:
        cout << *(float*)(data->data());
        break;
    case CHAR_ENUM: case VARCHAR_ENUM:
        cout << (char*)(data->data());
        break;
    default: assert(false);
    }
}

void BPlusTree::Debug(node_t* node)
{
    cout << "============ " << node->page_id << " ===========" << endl;
    cout << "size = " << node->size << endl;
    cout << "is_leaf = " << node->is_leaf << endl;
    for(int i = 0; i < node->size; i ++)
    {
        cout << "key = "; Output(key_file->Fetch(node->key_rids[i])); cout << " ";
        cout << "child = " << node->children_entries[i]; cout << " ";
        cout << "count = " << node->values_count[i]; cout << " ";
        cout << endl;
    }
    if (node->is_leaf) return;
    for(int i = 0; i < node->size; i ++)
    {
        Debug(LoadNode(node->children_entries[i]));
    }
}

void BPlusTree::SplitChild(node_t* node, int idx)
{
    node_t* a = LoadNode(node->children_entries[idx]);
    node_t* b = NewNode();

    b->is_leaf = a->is_leaf;
    b->size = a->size = N/2;
    for(int i = N/2; i < N; i ++)
    {
        b->key_rids[i-N/2] = a->key_rids[i];
        b->children_entries[i-N/2] = a->children_entries[i];
        b->values_count[i-N/2] = a->values_count[i];
    }
    for(int i = node->size; i > idx+1; i --)
    {
        node->key_rids[i] = node->key_rids[i-1];
        node->children_entries[i] = node->children_entries[i-1];
        node->values_count[i] = node->values_count[i-1];
    }
    if (a->is_leaf)
    {
        if (a->next_page_id != -1) LoadNode(a->next_page_id)->prev_page_id = b->page_id;
        b->next_page_id = a->next_page_id;
        a->next_page_id = b->page_id;
        b->prev_page_id = a->page_id;
        if (header->last_page_id == a->page_id) header->last_page_id = b->page_id;
    }
    int a_count = 0;
    for(int i = 0; i < N/2; i ++)
        a_count += a->values_count[i];
    int b_count = 0;
    for(int i = 0; i < N/2; i ++)
        b_count += b->values_count[i];
    
    data_t a_max_key = key_file->Fetch(a->key_rids[a->size-1]);
    node->key_rids[idx+1] = node->key_rids[idx];
    node->key_rids[idx] = key_file->Insert(a_max_key);
    node->children_entries[idx+1] = b->page_id;
    node->values_count[idx] = a_count;
    node->values_count[idx+1] = b_count;
    node->size ++;
}

void BPlusTree::InsertNotFull(node_t* node, data_t key, int value)
{
    int pos = 0; // 第一个>=key的位置
    while(pos < node->size && compare(type, key_file->Fetch(node->key_rids[pos]), key) < 0) pos ++;

    if (node->is_leaf)
    {
        if (pos < node->size && compare(type, key_file->Fetch(node->key_rids[pos]), key) == 0) // key相等
        {
            vector_t value_list = data_file->Fetch(node->children_entries[pos]); // 追加value
            value_list->push_back(value);
            node->children_entries[pos] = data_file->Save(node->children_entries[pos], value_list, true);
            node->values_count[pos] ++;
        } else {
            for(int i = node->size - 1; i >= pos; i --) // 插入
            {
                node->key_rids[i+1] = node->key_rids[i];
                node->children_entries[i+1] = node->children_entries[i];
                node->values_count[i+1] = node->values_count[i];
            }
            node->size ++;
            node->key_rids[pos] = key_file->Insert(key);
            node->children_entries[pos] = data_file->NewVector(value);
            node->values_count[pos] = 1;
        }
    } else {
        if (pos >= node->size) pos --;
        node_t* c = LoadNode(node->children_entries[pos]);
        if (c->size == N)
        {
            SplitChild(node, pos);
            if (compare(type, key, key_file->Fetch(node->key_rids[pos])) > 0) pos ++;
        }
        InsertNotFull(LoadNode(node->children_entries[pos]), key, value);
        node->values_count[pos] ++;
        if (pos == node->size-1)
        {
            if (compare(type, key, key_file->Fetch(node->key_rids[pos])) > 0)
            {
                key_file->Delete(node->key_rids[pos]);
                node->key_rids[pos] = key_file->Insert(key);
            }
        }
    }
}

void BPlusTree::MergeChildren(node_t* node, int idx)
{
    node_t* a = LoadNode(node->children_entries[idx]);
    node_t* b = LoadNode(node->children_entries[idx+1]);

    for(int i = 0; i < N/2; i ++)
    {
        a->key_rids[i+N/2] = b->key_rids[i];
        a->children_entries[i+N/2] = b->children_entries[i];
        a->values_count[i+N/2] = b->values_count[i];
    }
    a->size = N;
    a->next_page_id = b->next_page_id;
    if (a->next_page_id != -1) LoadNode(a->next_page_id)->prev_page_id = a->page_id;
    // FIXME 回收b

    key_file->Delete(node->key_rids[idx]);
    node->key_rids[idx] = node->key_rids[idx+1];
    node->values_count[idx] += node->values_count[idx+1];
    for(int i = idx+1; i+1 < node->size; i ++)
    {
        node->key_rids[i] = node->key_rids[i+1];
        node->children_entries[i] = node->children_entries[i+1];
        node->values_count[i] = node->values_count[i+1];
    }
    node->size --;
}

void BPlusTree::MoveChildBackward(node_t* node, int idx)
{
    node_t* a = LoadNode(node->children_entries[idx]);
    node_t* b = LoadNode(node->children_entries[idx+1]);

    for(int i = b->size-1; i >= 0; i --)
    {
        b->key_rids[i+1] = b->key_rids[i];
        b->children_entries[i+1] = b->children_entries[i];
        b->values_count[i+1] = b->values_count[i];
    }
    b->key_rids[0] = a->key_rids[a->size-1];
    b->children_entries[0] = a->children_entries[a->size-1];
    b->values_count[0] = a->values_count[a->size-1];
    node->values_count[idx] -= b->values_count[0];
    node->values_count[idx+1] += b->values_count[0];

    a->size --;
    data_t key = key_file->Fetch(a->key_rids[a->size-1]);
    key_file->Delete(node->key_rids[idx]);
    node->key_rids[idx] = key_file->Insert(key);
}

void BPlusTree::MoveChildForward(node_t* node, int idx)
{
    node_t* a = LoadNode(node->children_entries[idx]);
    node_t* b = LoadNode(node->children_entries[idx-1]);

    b->key_rids[b->size] = a->key_rids[0];
    b->children_entries[b->size] = a->children_entries[0];
    b->values_count[b->size] = a->values_count[0];
    for(int i = 0; i+1 < a->size; i ++)
    {
        a->key_rids[i] = a->key_rids[i+1];
        a->children_entries[i] = a->children_entries[i+1];
        a->values_count[i] = a->values_count[i+1];
    }
    node->values_count[idx] -= b->values_count[b->size];
    node->values_count[idx-1] += b->values_count[b->size];

    b->size ++;
    data_t key = key_file->Fetch(b->key_rids[b->size-1]);
    key_file->Delete(node->key_rids[idx-1]);
    node->key_rids[idx-1] = key_file->Insert(key);
}

void BPlusTree::DeleteLargeEnough(node_t* node, data_t key, int value)
{
    int pos = 0; // 第一个<=key的位置
    while(pos < node->size && compare(type, key_file->Fetch(node->key_rids[pos]), key) < 0) pos ++;
    assert(pos < node->size);

    if (node->is_leaf)
    {
        vector_t value_list = data_file->Fetch(node->children_entries[pos]);
        int value_pos = -1;
        for(int i = 0; i < (int)value_list->size(); i ++)
            if (value_list->at(i) == value)
            {
                value_pos = i;
                break;
            }
        if (value_pos == -1)
        {
            assert(false);
        } else {
            for(int i = value_pos; i+1 < (int)value_list->size(); i ++)
                value_list->data()[i] = value_list->data()[i+1];
            value_list->pop_back();
            if (value_list->size() > 0) {
                node->children_entries[pos] = data_file->Save(node->children_entries[pos], value_list);
                node->values_count[pos] --;
            } else {
                key_file->Delete(node->key_rids[pos]);
                for(int i = pos; i+1 < node->size; i ++)
                {
                    node->key_rids[i] = node->key_rids[i+1];
                    node->children_entries[i] = node->children_entries[i+1];
                    node->values_count[i] = node->values_count[i+1];
                }
                node->size --;
            }
        }
    } else {
        node_t* c = LoadNode(node->children_entries[pos]);
        if (c->size <= N/2)
        {
            if (pos > 0) // 向前
            {
                if (LoadNode(node->children_entries[pos-1])->size == N/2)
                {
                    MergeChildren(node, pos-1);
                    pos --;
                } else {
                    MoveChildBackward(node, pos-1);
                }
            } else {
                if (LoadNode(node->children_entries[pos+1])->size == N/2)
                {
                    MergeChildren(node, pos);
                } else {
                    MoveChildForward(node, pos+1);
                }
            }
        }
        node->values_count[pos] --;
        DeleteLargeEnough(LoadNode(node->children_entries[pos]), key, value);
    }
}

BPlusTree::search_rst BPlusTree::SearchGE(node_t* node, data_t key, int lt_count)
{
    int pos = 0;
    while(pos < node->size && compare(type, key_file->Fetch(node->key_rids[pos]), key) < 0) pos ++;
    for(int i = 0; i < pos; i ++) lt_count += node->values_count[i];
    if (pos >= node->size) return (search_rst){NULL, -1, lt_count, 0};

    if (node->is_leaf)
    {
        int eq_count = 0;
        if (compare(type, key_file->Fetch(node->key_rids[pos]), key) == 0)
            eq_count = data_file->Fetch(node->children_entries[pos])->size();
        return (search_rst){node, pos, lt_count, eq_count};
    }

    return SearchGE(LoadNode(node->children_entries[pos]), key, lt_count);
}

BPlusTree::search_rst BPlusTree::SearchLE(node_t* node, data_t key, int lt_count)
{
    int pos = node->size - 1;
    while(pos >= 0 && compare(type, key_file->Fetch(node->key_rids[pos]), key) > 0) pos --;
    for(int i = 0; i < pos; i ++) lt_count += node->values_count[i];
    if (pos < 0) return (search_rst){NULL, -1, lt_count, 0};

    if (node->is_leaf)
    {
        int eq_count = 0;
        if (compare(type, key_file->Fetch(node->key_rids[pos]), key) == 0)
            eq_count = data_file->Fetch(node->children_entries[pos])->size();
        return (search_rst){node, pos, lt_count, eq_count};
    }

    return SearchLE(LoadNode(node->children_entries[pos]), key, lt_count);
}
