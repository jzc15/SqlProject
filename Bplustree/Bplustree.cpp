#include "Bplustree.h"

Bplus_tree::Bplus_tree(string databaseName, string tableName, uchar type, uint PageNo, uint parent)
{
    key = new RecordBinary[N];
    son = new uint[N];
    this->type = type;
    this->leaf = true;
    this->parent = parent;
    this->PageNo = PageNo;
    this->databaseName = databaseName;
    this->tableName = tableName;
    key_cnt = 0;
}
Bplus_tree::~Bplus_tree()
{
    delete key;
    delete son;
}

void Bplus_tree::print()
{
    cout << "-----------bplusTree----------" << endl;
    cout << "type = " << type << endl;
    cout << "leaf = " << leaf << endl;
    cout << "PageNo = " << PageNo << endl;
    cout << "key_cnt = " << key_cnt << endl;
    cout << "parent = " << parent << endl;
    for (ushort i = 0; i < key_cnt; i++)
    {
        cout << "key[" << i << "].size = " << key[i].size << endl;
        for(ushort j = 0; j < key[i].size; j++)
            cout << "key[" << i << "].ptr[" << j << "] = " << key[i].ptr[j] << endl;
        
    }
    for (ushort i = 0; i < key_cnt; i++)
        cout << "son[" << i << "] = " << son[i] << endl;
    return ;
}
void Bplus_tree::load()
{
    RecordBinary data;
    RecordManager rm(databaseName);
    data = rm.searchRecord(tableName, PageNo, 1);

    this->type = data.ptr[0];
    if (data.ptr[1] == 0)
        this->leaf = false;

    ushort *dushort;
    dushort = (ushort *)data.ptr;
    key_cnt = dushort[5];

    uint *duint;
    duint = (uint *)data.ptr + 2;
    PageNo = duint[0];
    parent = duint[1];

    int z = 10;
    for (ushort i = 0; i < key_cnt; i++)
    {
        dushort = (ushort *)data.ptr + z;
        key[i].size = dushort[0];
        z += 2;
        for (ushort j = 0; j < key[i].size; j++)
            key[i].ptr[j] = data.ptr[z++];
    }
    duint = (uint *)data.ptr + z;
    for (ushort i = 0; i < key_cnt; i++)
        son[i] = duint[i];
    return;
}
void Bplus_tree::save()
{
    RecordBinary data;
    data.ptr = new uchar[8184]; // 8192个空， 前8位为页头
    data.size = 8184;
    RecordManager rm(databaseName);

    data.ptr[0] = type;
    data.ptr[1] = leaf;

    ushort *dushort;
    dushort = (ushort *)data.ptr;
    dushort[3] = key_cnt;

    uint *duint;
    duint = (uint *)data.ptr + 2;
    duint[0] = PageNo;
    duint[1] = parent;
    
    int z = 10;
    for (ushort i = 0; i < key_cnt; i++)
    {
        dushort = (ushort *)data.ptr + z;
        dushort[0] = key[i].size;
        z += 2;
        for (ushort j = 0; j < key[i].size; j++)
            data.ptr[z++] = key[i].ptr[j];
    }
    duint = (uint *)data.ptr + z;
    for (ushort i = 0; i < key_cnt; i++)
        duint[i] = son[i];
    rm.createRecord(tableName, data);
    return;
}
bool Bplus_tree::compare(RecordBinary &A, RecordBinary &B)
{
    if (type == 1)
    {
        int *a, *b;
        a = (int *)A.ptr;
        b = (int *)B.ptr;
        return a[0] < b[0];
    }
    if (type == 2)
    {
        float *a, *b;
        a = (float *)A.ptr;
        b = (float *)B.ptr;
        return a[0] < b[0];
    }

    return A < B;
}
void Bplus_tree::insert(RecordBinary &a, uint rid, bool merge)
{
    if (leaf == true || merge == true)
    {
        if (key_cnt < N) //插入
        {
            for (uint i = key_cnt - 1; i >= 0; i--)
            {
                if (compare(a, key[i]))
                {
                    key[i + 1] = key[i];
                    son[i + 1] = son[i];
                    continue;
                }
                key[i + 1] = a;
                son[i + 1] = rid;
                save();
                return;
            }
            key[0] = a;
            son[0] = rid;
            save();
            return;
        }
        //分裂，会导致某些节点堆父亲指针没有更新，由于更新代价过大，在以后树递归的时候维护


        //获取总共有多少个节点
        RecordManager rm(databaseName);
        ushort totalPage = rm.getPageCnt(tableName);
        Bplus_tree s2(databaseName, tableName, type, totalPage+1, parent);
        Bplus_tree s1(databaseName, tableName, type, totalPage+2, parent);
        //分裂操作
        RecordBinary mid = key[N/2];

        s1.key_cnt = N/2;
        key_cnt = N/2;
        s2.key_cnt = N-N/2;
        s2.leaf = leaf;

        if(PageNo == 1)
            for(ushort i = 0; i < N/2; i++)
                s1.key[i] = key[i], s1.son[i] = son[i];

        for(ushort i = N/2; i < N; i++)
            s2.key[i] = key[i], s2.son[i] = son[i];
        if(compare(a, mid))
        {
            if(PageNo == 1)
                s1.insert(a, rid, merge);
            else
                insert(a, rid, merge);
        }
        else
            s2.insert(a, rid, merge);
        
        //保存
        if(PageNo == 1)
        {
            s1.save();
            key_cnt = 2;
            key[0] = s1.key[0];
            son[0] = totalPage+2;
            key[1] = mid;
            son[1] = totalPage+1;
            leaf = false;
        }
        save();
        s2.save();

        //往上回溯
        if(PageNo == 1)
            return ;
        Bplus_tree fa(databaseName, tableName, type, parent, PageNo);
        fa.load();
        fa.insert(mid, totalPage, 1);
        return;
    }
    if (compare(a, key[0]))
    {
        key[0] = a;
        save();
    }
    //由于分裂操作维护父亲指针代价过大，采用每次递归时维护

    for (int i = 1; i < key_cnt; i++)
        if (compare(a, key[i]))
        {
            Bplus_tree b(databaseName, tableName, type, son[i - 1], PageNo);
            b.load();
            if(b.parent != PageNo)
            {
                b.parent = PageNo;
                b.save();
            }
            b.insert(a, rid, 0);
            
            return;
        }
    Bplus_tree b(databaseName, tableName, type, son[key_cnt - 1], PageNo);
    b.load();
    if(b.parent != PageNo)
    {
        b.parent = PageNo;
        b.save();
    }
    b.insert(a, rid, 0);
    return;
}