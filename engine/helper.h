#ifndef ENGINE_HELPER_H
#define ENGINE_HELPER_H

#include "crudop.h"
#include <ddf/record.h>
#include <iostream>
#include <vector>
using namespace std;

#define TITLE(name) {cout << "========= " << #name << " =========" << endl;}

// 检查该记录是否合法, rid表示当前record原来的rid，如果是新记录rid=-1
bool record_check_ok(Record::ptr record, int rid = -1);
// 类型检查
bool value_type_check_ok(type_t type, Value::ValueType value_type);
// 将该记录从索引中移除
void remove_record_from_indices(Record::ptr record, int rid);
// 在索引中添加记录
void insert_record_to_indices(Record::ptr record, int rid);

#define INF int(1e9)
// 检查是否符合条件, columnIndex表示cond.column列坐标
bool test_condition(Record::ptr record, const Condition& cond, int columnIndex = -1);
// 两个表之间检查, cond.column是第一个表, cond.expr.column是第二个表
bool test_condition(Record::ptr record_a, Record::ptr record_b, const Condition& cond);
// 统计满足条件的数量，无法统计则返回INF
int calculate_condition_count(TableDesc::ptr td, const Condition& cond);
// 获取满足条件的rid列表
vector<int> list_condition_rids(TableDesc::ptr td, const Condition& cond);
// 获取满足条件的rid列表
vector<int> list_conditions_rids(TableDesc::ptr td, const vector<Condition>& conds);

void solve_column_tb_name(Context* ctx, const vector<string>& tables, Column& column);

int search_in_primary(TableDesc::ptr td, data_t key);

#endif // ENGINE_HELPER_H