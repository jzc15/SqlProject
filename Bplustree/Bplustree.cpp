#include "Bplustree.h"

Bplus_tree::Bplus_tree(string databaseName, string tableName, uchar type, uint PageNo, uint parent)
{
    key = new RecordBinary[N];
    son = new uint[N];
    this->type = type;
    this->leaf = true;
    this->left = 1;
    this->right = 1;
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
    cout << "-----------bplusTree-------------" << endl;
    cout << "type = " << (ushort)type << endl;
    cout << "leaf = " << leaf << endl;
    cout << "PageNo = " << PageNo << endl;
    cout << "key_cnt = " << key_cnt << endl;
    cout << "left = " << left << endl;
    cout << "right = " << right << endl;
    cout << "parent = " << parent << endl;
    for (ushort i = 0; i < key_cnt; i++)
    {
        cout << "key[" << i << "].size = " << key[i].size << endl;
        for(ushort j = 0; j < key[i].size; j++)
            cout << "key[" << i << "].ptr[" << j << "] = " << (ushort)key[i].ptr[j] << endl;
        
    }
    for (ushort i = 0; i < key_cnt; i++)
        cout << "son[" << i << "] = " << son[i] << endl;
    cout << "-----------bplusTree----end------" << endl;
    return ;
}
void Bplus_tree::load()
{
    RecordBinary data;
    RecordManager rm(databaseName);

    //cout << rm.getPageCnt(tableName) << endl;
    data = rm.searchRecord(tableName, PageNo, 1);
    //cout << 111 << endl;
    //cout << data.size << endl;
    this->type = data.ptr[0];
    if (data.ptr[1] == 0)
        this->leaf = false;

    uint *duint;
    duint = (uint *)(data.ptr + 2);
    PageNo = duint[0];
    parent = duint[1];
    left = duint[2];
    right = duint[3];

    ushort *dushort;
    dushort = (ushort *)(data.ptr+18);
    key_cnt = dushort[0];

    //cout << "load key_cnt = " << key_cnt << endl;

    uint z = 20;
    for (ushort i = 0; i < key_cnt; i++)
    {
        dushort = (ushort *)(data.ptr + z);
        key[i].size = dushort[0];
        key[i].ptr = new uchar[key[i].size];
        //cout << "key[" << i << "] = " << key[i].size << endl; 
        z += 2;
        for (ushort j = 0; j < key[i].size; j++)
            key[i].ptr[j] = data.ptr[z++];
    }
    duint = (uint *)(data.ptr + z);
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

    uint *duint;
    duint = (uint *)(data.ptr + 2);
    duint[0] = PageNo;
    duint[1] = parent;
    duint[2] = left;
    duint[3] = right;
    
    ushort *dushort;
    dushort = (ushort *)(data.ptr+18);
    dushort[0] = key_cnt;
    //dushort = (ushort *)(data.ptr);
    //cout << "save key_cnt = " << dushort[5] << endl;

    //cout << "save PageNo = " << PageNo << endl;
    //cout << key[0].size << endl;
    uint z = 20;
    for (ushort i = 0; i < key_cnt; i++)
    {
        dushort = (ushort *)(data.ptr + z);
        dushort[0] = key[i].size;
        z += 2;
        for (ushort j = 0; j < key[i].size; j++)
            data.ptr[z++] = key[i].ptr[j];
    }
    duint = (uint *)(data.ptr + z);
    for (ushort i = 0; i < key_cnt; i++)
        duint[i] = son[i];
    
    rm.saveRecord(tableName, data, PageNo, 1);
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
        //cout << "insert PageNo = " << PageNo << ", merge = " << merge << endl;

        //cout << (int)a.ptr[0] << endl;
        if (key_cnt < N) //插入 
        {
            for (int i = key_cnt - 1; i >= 0; i--)
            {
                if (compare(a, key[i]))
                {
                    key[i + 1] = key[i];
                    son[i + 1] = son[i];
                    continue;
                }

                key_cnt++;
                key[i+1] = a;
                son[i+1] = rid;
                save();
                return;
            }
            key_cnt++;
            //cout << key_cnt << endl;
            key[0] = a;
            //cout << key_cnt << endl;
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
        
        s2.right = right;
        if(PageNo == 1)
        {
            s2.left = totalPage+2;
            s1.right = totalPage+1;
            s1.left = 1;
            s2.right = 1;
            left = totalPage+1;
            right = totalPage+2;
        }
        else
        {
            s2.left = PageNo;
            s2.right = right;
            right = totalPage+1;
        }
        s1.key_cnt = N/2;
        key_cnt = N/2;
        s2.key_cnt = N-N/2;
        s2.leaf = leaf;
        
        for(ushort i = N/2; i < N; i++)
            s2.key[i-N/2] = key[i], s2.son[i-N/2] = son[i];

        //s2.save();
        if(PageNo == 1)
        {
            for(ushort i = 0; i < N/2; i++)
                s1.key[i] = key[i], s1.son[i] = son[i];
            //s1.save();
        }
        
        if(compare(a, mid))
        {
            if(PageNo == 1)
            {
                s1.insert(a, rid, merge);
                //s1.save();
            }
            else
                insert(a, rid, merge);
            
        }
        else
        {
            s2.insert(a, rid, merge);
            //s2.save();
        }
        //cout << "s2.pageNo = " << s2.PageNo << endl;
        s2.save();
        if(PageNo == 1)
            s1.save();
        save();
        //保存
        if(PageNo == 1)
        {
            //cout << "right" << endl;
            key_cnt = 2;
            key[0] = s1.key[0];
            son[0] = totalPage+2;
            key[1] = mid;
            son[1] = totalPage+1;
            leaf = false;
            save();
        }
        
        
        //往上回溯
        if(PageNo == 1)
            return ;
        Bplus_tree fa(databaseName, tableName, type, parent, PageNo);
        fa.load();
        cout << "totalPage = " << totalPage << endl;
        fa.insert(mid, totalPage+1, 1);
        return;
    }
    if (compare(a, key[0]))
    {
        key[0] = a;
        save();
    }
    //由于分裂操作维护父亲指针代价过大，采用每次递归时维护
    /*
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
    */
    int k = 1 << 8;
    int z = 0;
    //倍增查找
    while(k > 0)
    {
        if( k+z < key_cnt && (!compare(a, key[z+k])))
            z += k;
        k >>= 1;
    }
    Bplus_tree btree(databaseName, tableName, type, son[z], PageNo);
    btree.load();

     //由于分裂操作维护父亲指针代价过大，采用每次递归时维护
    if(btree.parent != PageNo)
    {
        btree.parent = PageNo;
        btree.save();
    }
    btree.insert(a, rid, 0);
    return;
}

uint Bplus_tree::getRid(ushort pn, ushort sn)
{
    uint pni = pn;
    uint sni = sn;
    return (pni<<16)+sni;
}
void Bplus_tree::getPnSn(uint rid, ushort &pn, ushort &sn)
{
    pn = rid >> 16;
    sn = rid % 65536;
    return ;
}
uint Bplus_tree::search(RecordBinary &a, ushort &pn, int &z)
{
    int k = 1 << 8;
    //倍增查找
    while(k > 0)
    {
        if( k+z < key_cnt && (!compare(a, key[z+k])))
            z += k;
        k >>= 1;
    }
    if(leaf)
        return son[z];
    Bplus_tree btree(databaseName, tableName, type, son[z], PageNo);
    btree.load();

     //由于分裂操作维护父亲指针代价过大，采用每次递归时维护
    if(btree.parent != PageNo)
    {
        btree.parent = PageNo;
        btree.save();
    }
   return btree.search(a);
}
uint Bplus_tree::search(RecordBinary &a)
{
    ushort pn;
    int z = 0;
    return search(a, pn, z);
}
void Bplus_tree::remove(RecordBinary &a)
{
    int k = 1 << 8;
    int z = 0;
    //倍增查找
    while(k > 0)
    {
        if( k+z < key_cnt && (!compare(a, key[z+k])))
            z += k;
        k >>= 1;
    }
    if(leaf)                      //如果删除了最小值，由于非叶节点只作为分界点存在，因此无需维护父节点
    {
        for(int i = z+1; i < key_cnt; i++)
            key[i-1] = key[i], son[i-1] = son[i];
        key_cnt--;
        
        if(key_cnt < N/2)
        {
            Bplus_tree btree(databaseName, tableName, type, parent, PageNo);
            btree.load();
            bool hleft, hright;
        
            hleft = (!compare(btree.key[0], a)) && left != 1;        
            hright = (!compara(a, btree.key[key_cnt-1])) && right != 1;

            Bplus_tree bleft(databaseName, tableName, type, left, PageNo);   
            Bplus_tree bright(databaseName, tableName, type, right, PageNo);   

            if(hleft)
            {
                bleft.load();
                if(bleft.key_cnt > N/2)
                {

                    return ;
                }
            }

            if(hright)
            {
                bright.load();
                if(bright.key_cnt > N/2)
                {
                    
                    return ;
                }
            }

            if(hleft)
            {
                return ;
            }

            if(hright)
            {
                return ;
            }
        }
        return ;
    }
    Bplus_tree btree(databaseName, tableName, type, son[z], PageNo);
    btree.load();

     //由于分裂操作维护父亲指针代价过大，采用每次递归时维护
    if(btree.parent != PageNo)
    {
        btree.parent = PageNo;
        btree.save();
    }
   return btree.remove(a);
}