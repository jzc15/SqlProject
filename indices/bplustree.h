#ifndef INDICES_BPLUSTREE_H
#define INDICES_BPLUSTREE_H

#include <disk/common.h>
#include <disk/file.h>
#include <ddf/typeinfo.h>
#include <datamanager/slotsfile.h>
#include <memory>
#include <string>

using namespace std;

/**
 * B+树
 * 第0页是头
 * 关键字存其对应儿子的最大值
 * */
class BPlusTree
{
private:
    const static int N = 4;
    enum page_t { PAGE_NONE = 0, PAGE_HEADER, PAGE_NODE };
    struct header_t
    {
        page_t page_type;
        int root_page_id;
        int first_page_id;
        int last_page_id;
        int total_count; // value总数
    };
    struct node_t
    {
        page_t page_type;
        int page_id;
        bool is_leaf;
        int next_page_id; // 叶节点的下一个, -1表示没有
        int prev_page_id; // 叶节点的上一个, -1表示没有
        int size;
        int key_rids[N]; // 关键字
        int children_entries[N]; // 儿子 / 数据rid
        int values_count[N]; // 对应儿子包含的value总数
    };
    struct search_rst // 搜索节点, node=NULL表示没有
    {
        node_t* node;
        int pos;
        int lt_count; // 小于的数量
        int eq_count; // 等于的数量
    };
public:
    typedef shared_ptr<BPlusTree> ptr;
    class Iterator
    {
    public:
        data_t Key();
        int Value();

        void Prev();
        void Next();
        bool End();
    
    private:
        BPlusTree* tree;
        node_t* node;
        int key_pos;
        int value_pos;

        Iterator(BPlusTree* tree, node_t* node, int key_pos, int value_pos);

        friend BPlusTree;
    };

    BPlusTree(const string& filename, type_t type);
    ~BPlusTree();

    void Debug();

    // 插入
    void Insert(data_t key, int value);

    // 删除
    void Delete(data_t key, int value);

    // 键是否存在
    bool IsKeyExists(data_t key);

    // 总数
    int TotalCount();

    // 小于的数量
    int LTCount(data_t key);

    // 等于的数量
    int EQCount(data_t key);

    // 小于等于的数量
    int LECount(data_t key);

    Iterator Begin();

private:
    const string filename;
    const type_t type;
    const bool type_fixed;
    File::ptr b_file;
    SlotsFile::ptr data_file;

    header_t* header;

    void Init();

    node_t* NewNode();
    node_t* LoadNode(int page);

    void Output(data_t data);

    void Debug(node_t* node);

    // a - b
    int Compare(data_t a, data_t b);

    // 将node的地idx个儿子(满的)分裂成两个，node未满
    void SplitChild(node_t* node, int idx);

    // 插入，node节点未满
    void InsertNotFull(node_t* node, data_t key, int value);

    // 合并idx和idx+1，向前移动
    void MergeChildren(node_t* node, int idx);

    // 将idx的最后一个结点后移
    void MoveChildBackward(node_t* node, int idx);

    // 将idx的第一个结点前移
    void MoveChildForward(node_t* node, int idx);

    // 删除，node节点删除一个之后大于N/2
    void DeleteLargeEnough(node_t* node, data_t key, int value);

    // 搜索第一个大于等于key
    search_rst SearchGE(node_t* node, data_t key, int lt_count = 0);

    // 搜索最后一个小于等于key
    search_rst SearchLE(node_t* node, data_t key, int lt_count = 0);
};

#endif // INDICES_BPLUSTREE_H