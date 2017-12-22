#include "helper.h"
#include <disk/common.h>
#include <datamanager/slotsfile.h>
#include <indices/bplustree.h>
#include <iostream>
#include <cassert>

using namespace std;

// 检查该记录是否合法
bool record_check_ok(Record::ptr record, int rid)
{
    for(int i = 0; i < (int)record->td->cols.size(); i ++)
    {
        auto c = record->td->cols[i];
        if (!c->allow_null && record->IsNull(i)) // 空值检查
        {
            cerr << "column `" << c->columnName << "` not allow null" << endl;
            return false;
        }
        if (c->is_primary) // 主键唯一
        {
            BPlusTree::ptr indices = make_shared<BPlusTree>(c->PrimaryFilename(), c->typeEnum);
            if (indices->EQCount(record->GetValue(i)) > 0)
            {
                cerr << "primary column `" << c->columnName << "` value exists : " << stringify(c->typeEnum, record->GetValue(i)) << endl;
                return false;
            }
        }
        if (c->is_foreign_key) // 外键存在
        {
            auto foreign_td = record->td->dd->SearchTable(c->foreign_tb_name);
            auto foreign_cd = foreign_td->Column(c->foreign_col_name);
            BPlusTree::ptr indices = make_shared<BPlusTree>(foreign_cd->PrimaryFilename(), foreign_cd->typeEnum);
            if (indices->EQCount(record->GetValue(i)) == 0)
            {
                cerr << "foreign column `" << c->columnName << "` for `" << c->foreign_tb_name << "." << c->foreign_col_name << "` key not exists : " << stringify(c->typeEnum, record->GetValue(i)) << endl;
                return false;
            }
        }
    }
    return true;
}
// 类型检查
bool value_type_check_ok(type_t type, Value::ValueType value_type)
{
    if (value_type == Value::VALUE_NULL) return true;
    switch(type)
    {
    case INT_ENUM:
        return value_type == Value::VALUE_INT;
    case CHAR_ENUM: case VARCHAR_ENUM:
        return value_type == Value::VALUE_STRING;
    default:
        assert(false);
    }
}
// 将该记录从索引中移除
void remove_record_from_indices(Record::ptr record, int rid)
{
    for(int i = 0; i < (int)record->td->cols.size(); i ++)
    {
        auto c = record->td->cols[i];
        if (c->indexed && !record->IsNull(i))
        {
            BPlusTree::ptr indices = make_shared<BPlusTree>(c->IndexFilename(), c->typeEnum);
            indices->Delete(record->GetValue(i), rid);
        }
        if (c->is_primary && !record->IsNull(i)) // 主键
        {
            BPlusTree::ptr indices = make_shared<BPlusTree>(c->PrimaryFilename(), c->typeEnum);
            indices->Delete(record->GetValue(i), rid);
        }
    }
}
// 在索引中添加记录
void insert_record_to_indices(Record::ptr record, int rid)
{
    for(int i = 0; i < (int)record->td->cols.size(); i ++)
    {
        auto c = record->td->cols[i];
        if (c->indexed && !record->IsNull(i))
        {
            BPlusTree::ptr indices = make_shared<BPlusTree>(c->IndexFilename(), c->typeEnum);
            indices->Insert(record->GetValue(i), rid);
        }
        if (c->is_primary && !record->IsNull(i)) // 主键
        {
            BPlusTree::ptr indices = make_shared<BPlusTree>(c->PrimaryFilename(), c->typeEnum);
            indices->Insert(record->GetValue(i), rid);
        }
    }
}

bool test_condition(Record::ptr record, const Condition& cond, int columnIndex)
{
    if (columnIndex < 0) columnIndex = record->td->ColumnIndex(cond.column.col_name);
    auto cd = record->td->Column(columnIndex);
    data_t data_a = record->GetValue(columnIndex);
    data_t data_b;
    if (cond.is_binary_operator())
    {
        switch(cond.expr.expr_type)
        {
        case Expr::EXPR_COLUMN:
            data_b = record->GetValue(cond.expr.column.col_name);
            break;
        case Expr::EXPR_VALUE:
            data_b = cond.expr.value.data;
            break;
        default: assert(false);
        }
    }
    switch(cond.op)
    {
    case Condition::OP_EQ:
        return compare(cd->typeEnum, data_a, data_b) == 0;
        break;
    case Condition::OP_NEQ:
        return compare(cd->typeEnum, data_a, data_b) != 0;
        break;
    case Condition::OP_LT:
        return compare(cd->typeEnum, data_a, data_b) < 0;
        break;
    case Condition::OP_GT:
        return compare(cd->typeEnum, data_a, data_b) > 0;
        break;
    case Condition::OP_LE:
        return compare(cd->typeEnum, data_a, data_b) <= 0;
        break;
    case Condition::OP_GE:
        return compare(cd->typeEnum, data_a, data_b) >= 0;
        break;
    case Condition::OP_IS_NULL:
        return data_a == nullptr;
        break;
    case Condition::OP_NOT_NULL:
        return data_a != nullptr;
        break;
    default: assert(false);
    }

    return false;
}

bool test_condition(Record::ptr record_a, Record::ptr record_b, const Condition& cond)
{
    auto cd = record_a->td->Column(cond.column.col_name);
    data_t data_a = record_a->GetValue(record_a->td->ColumnIndex(cond.column.col_name));
    data_t data_b = record_b->GetValue(record_b->td->ColumnIndex(cond.expr.column.col_name));
    switch(cond.op)
    {
    case Condition::OP_EQ:
        return compare(cd->typeEnum, data_a, data_b) == 0;
        break;
    case Condition::OP_NEQ:
        return compare(cd->typeEnum, data_a, data_b) != 0;
        break;
    case Condition::OP_LT:
        return compare(cd->typeEnum, data_a, data_b) < 0;
        break;
    case Condition::OP_GT:
        return compare(cd->typeEnum, data_a, data_b) > 0;
        break;
    case Condition::OP_LE:
        return compare(cd->typeEnum, data_a, data_b) <= 0;
        break;
    case Condition::OP_GE:
        return compare(cd->typeEnum, data_a, data_b) >= 0;
        break;
    default: assert(false);
    }
    return false;
}

int calculate_condition_count(TableDesc::ptr td, const Condition& cond)
{
    ColDesc::ptr cd = td->Column(cond.column.col_name);

    string indices_filename;
    if (cd->indexed) indices_filename = cd->IndexFilename();
    if (cd->is_primary) indices_filename = cd->PrimaryFilename();

    if (cd->indexed || cd->is_primary)
    {
        if (cond.expr.expr_type == Expr::EXPR_VALUE)
        {
            if (
                cond.op == Condition::OP_EQ ||
                cond.op == Condition::OP_NEQ ||
                cond.op == Condition::OP_LT ||
                cond.op == Condition::OP_GT ||
                cond.op == Condition::OP_LE ||
                cond.op == Condition::OP_GE)
            {
                BPlusTree::ptr indices = make_shared<BPlusTree>(indices_filename, cd->typeEnum);
                data_t key = cond.expr.value.data;
                int count = 0;
                switch(cond.op)
                {
                case Condition::OP_EQ:
                    count = indices->EQCount(key);
                    break;
                case Condition::OP_NEQ:
                    count = indices->TotalCount() - indices->EQCount(key);
                    break;
                case Condition::OP_LT:
                    count = indices->LTCount(key);
                    break;
                case Condition::OP_GT:
                    count = indices->TotalCount() - indices->LECount(key);
                    break;
                case Condition::OP_LE:
                    count = indices->LECount(key);
                    break;
                case Condition::OP_GE:
                    count = indices->TotalCount() - indices->LTCount(key);
                    break;
                default: assert(false);
                }
                return count;
            }
        }
    }

    return INF;
}

vector<int> list_condition_rids(TableDesc::ptr td, const Condition& cond)
{
    ColDesc::ptr cd = td->Column(cond.column.col_name);

    string indices_filename;
    if (cd->indexed) indices_filename = cd->IndexFilename();
    if (cd->is_primary) indices_filename = cd->PrimaryFilename();

    if (cd->indexed || cd->is_primary)
    {
        if (cond.expr.expr_type == Expr::EXPR_VALUE)
        {
            if (
                cond.op == Condition::OP_EQ ||
                cond.op == Condition::OP_NEQ ||
                cond.op == Condition::OP_LT ||
                cond.op == Condition::OP_GT ||
                cond.op == Condition::OP_LE ||
                cond.op == Condition::OP_GE)
            {
                BPlusTree::ptr indices = make_shared<BPlusTree>(indices_filename, cd->typeEnum);
                data_t key = cond.expr.value.data;
                vector<int> ans;
                switch(cond.op)
                {
                case Condition::OP_EQ:
                    {
                        auto iter = indices->Lower(key);
                        while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) == 0)
                        {
                            ans.push_back(iter.Value());
                            iter.Next();
                        }
                    }
                    break;
                case Condition::OP_NEQ:
                    {
                        auto iter = indices->Begin();
                        while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) < 0)
                        {
                            ans.push_back(iter.Value());
                            iter.Next();
                        }
                        iter = indices->Upper(key);
                        while(!iter.End())
                        {
                            ans.push_back(iter.Value());
                            iter.Next();
                        }
                    }
                    break;
                case Condition::OP_LT:
                    {
                        auto iter = indices->Begin();
                        while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) < 0)
                        {
                            ans.push_back(iter.Value());
                            iter.Next();
                        }
                    }
                    break;
                case Condition::OP_GT:
                    {
                        auto iter = indices->Upper(key);
                        while(!iter.End())
                        {
                            ans.push_back(iter.Value());
                            iter.Next();
                        }
                    }
                    break;
                case Condition::OP_LE:
                    {
                        auto iter = indices->Begin();
                        while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) <= 0)
                        {
                            ans.push_back(iter.Value());
                            iter.Next();
                        }
                    }
                    break;
                case Condition::OP_GE:
                    {
                        auto iter = indices->Lower(key);
                        while(!iter.End())
                        {
                            ans.push_back(iter.Value());
                            iter.Next();
                        }
                    }
                    break;
                default: assert(false);
                }
                return ans;
            }
        }
    }

    vector<int> ans;
    SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
    const int columnIndex = td->ColumnIndex(cond.column.col_name);
    data_t data = file->Begin();
    while(data != nullptr)
    {
        Record::ptr record = td->RecoverRecord(data);
        if (test_condition(record, cond, columnIndex)) ans.push_back(file->CurrentRID());
        data = file->Next();
    }

    return ans;
}

vector<int> list_conditions_rids(TableDesc::ptr td, const vector<Condition>& conds)
{
    if ((int)conds.size() == 0)
    {
        vector<int> ans;
        SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
        data_t data = file->Begin();
        while(data != nullptr)
        {
            ans.push_back(file->CurrentRID());
            data = file->Next();
        }
        return ans;
    }

    int bestColumnIndex = -1, bestCount = INF + 1;
    for(int i = 0; i < (int)conds.size(); i ++)
    {
        int count = calculate_condition_count(td, conds[i]);
        cout << "cond " << i << " : " << count << endl;
        if (count < bestCount)
        {
            bestCount = count;
            bestColumnIndex = i;
        }
    }
    vector<int> rids = list_condition_rids(td, conds[bestColumnIndex]);
    cout << "best rid count : " << rids.size() << endl;
    SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
    vector<int> ans;
    for(auto rid : rids)
    {
        bool flag = true;
        Record::ptr record = td->RecoverRecord(file->Fetch(rid));
        for(int i = 0; i < (int)conds.size() && flag; i ++)
            if (i != bestColumnIndex)
                flag &= test_condition(record, conds[i]);
        if (flag) ans.push_back(rid);
    }
    return ans;
}

void solve_column_tb_name(Context* ctx, const vector<string>& tables, Column& column)
{
    if (!column.col_name_only) return;
    TableDesc::ptr match_td = nullptr;
    for(const auto& tb : tables)
    {
        auto td = ctx->dd->SearchTable(tb);
        if (td->IsColumnExists(column.col_name))
        {
            if (match_td != nullptr)
            {
                cerr << "multi table match for column `" << column.col_name << "`" << endl;
            }
            assert(match_td == nullptr);
            match_td = td;
        }
    }
    if (match_td == nullptr)
    {
        cerr << "no table match column `" << column.col_name << "`" << endl;
    }
    assert(match_td != nullptr);
    column.tb_name = match_td->tableName;
}

int search_in_primary(TableDesc::ptr td, data_t key)
{
    assert(td->PrimaryKeyIdx() != -1);
    BPlusTree::ptr indices = make_shared<BPlusTree>(td->Column(td->PrimaryKeyIdx())->PrimaryFilename(), td->Column(td->PrimaryKeyIdx())->typeEnum);
    auto iter = indices->Lower(key);
    if (iter.End() || compare(td->Column(td->PrimaryKeyIdx())->typeEnum, key, iter.Key()) != 0) return -1;
    return iter.Value();
}
