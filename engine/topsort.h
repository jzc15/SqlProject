#ifndef ENGINE_TOPSORT_H
#define ENGINE_TOPSORT_H

#include <set>
#include <map>
#include <vector>
#include <memory>
#include <queue>

using namespace std;

template<typename T>
class TopSort
{
public:
    TopSort();
    ~TopSort();

    // 建点
    void TouchNode(const T& x);
    // 建边
    void Build(const T& a, const T& b);
    // 拓扑排序，无法排序返回nullptr
    shared_ptr<vector<T>> Sort() const;

private:
    int n;
    map<T, int> ids; // 映射
    map<int, T> iids; // 逆映射
    vector<vector<int>> G;

    int GetID(const T& x);
};

template<typename T>
TopSort<T>::TopSort()
{
    n = 0;
}

template<typename T>
TopSort<T>::~TopSort()
{
}

template<typename T>
int TopSort<T>::GetID(const T& x)
{
    if (ids.find(x) == ids.end())
    {
        ids[x] = n;
        iids[n] = x;
        G.push_back(vector<int>());
        n ++;
    }
    return ids[x];
}

template<typename T>
void TopSort<T>::TouchNode(const T& x)
{
    GetID(x);
}

template<typename T>
void TopSort<T>::Build(const T& a, const T& b)
{
    G[GetID(a)].push_back(GetID(b));
}

template<typename T>
shared_ptr<vector<T>> TopSort<T>::Sort() const
{
    shared_ptr<vector<T>> ans = make_shared<vector<T>>();
    vector<int> du(n, 0);

    for(int u = 0; u < n; u ++)
    {
        for(int i = 0; i < (int)G[u].size(); i ++)
            du[G[u][i]] ++;
    }
    queue<int> que;
    for(int u = 0; u < n; u ++)
        if (!du[u]) que.push(u);
    while(!que.empty())
    {
        int u = que.front(); que.pop();
        ans->push_back(iids.at(u));
        for(int i = 0; i < (int)G[u].size(); i ++)
        {
            du[G[u][i]] --;
            if (!du[G[u][i]]) que.push(G[u][i]);
        }
    }
    if ((int)ans->size() != n) return nullptr;
    return ans;
}

#endif // ENGINE_TOPSORT_H