#include "helper.h"
#include <disk/common.h>
#include <datamanager/slotsfile.h>
#include <indices/bplustree.h>
#include <indices/hashtable.h>
#include <indices/multihashtable.h>
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
        if (c->is_foreign_key) // 外键存在
        {
            auto foreign_td = record->td->dd->SearchTable(c->foreign_tb_name);
            auto foreign_cd = foreign_td->Column(c->foreign_col_name);
            HashTable::ptr indices = make_shared<HashTable>(foreign_td->PrimaryFilename(), foreign_td->PrimarySize());
            if (!indices->Exists(record->GetValue(i)))
            {
                cerr << "foreign column `" << c->columnName << "` for `" << c->foreign_tb_name << "." << c->foreign_col_name << "` key not exists : " << stringify(c->typeEnum, record->GetValue(i)) << endl;
                return false;
            }
        }
    }
    if (record->td->PrimaryIdxs()->size() > 0u) // 主键唯一
    {
        HashTable::ptr indices = make_shared<HashTable>(record->td->PrimaryFilename(), record->td->PrimarySize());
        if (indices->Exists(record->PrimaryKey()) && indices->Fetch(record->PrimaryKey()) != rid)
        {
            cerr << "primary key exists : ";
            for(int i = 0; i < (int)record->td->PrimaryIdxs()->size(); i ++)
            {
                cout << stringify(record->td->Column(record->td->PrimaryIdxs()->at(i))->typeEnum, record->GetValue(i)) << " ";
            }
            cout << endl;
            return false;
        }
    }
    return true;
}
// 类型检查
bool value_type_trans_ok(type_t type, Value& value)
{
    if (value.value_type == Value::VALUE_NULL) return true;
    switch(type)
    {
    case INT_ENUM:
        return value.value_type == Value::VALUE_INT;
    case CHAR_ENUM: case VARCHAR_ENUM:
        return value.value_type == Value::VALUE_STRING;
    case DATE_ENUM:
        if (value.value_type == Value::VALUE_STRING)
        {
            value.string_to_date();
            return true;
        } else {
            return false;
        }
    case FLOAT_ENUM:
        if (value.value_type == Value::VALUE_FLOAT)
        {
            return true;
        } else if (value.value_type == Value::VALUE_INT)
        {
            value.int_to_float();
            return true;
        } else {
            return false;
        }
    case DECIMAL_ENUM:
        if (value.value_type == Value::VALUE_INT)
        {
            value.int_to_decimal();
            return true;
        } else if (value.value_type == Value::VALUE_FLOAT)
        {
            value.float_to_decimal();
            return true;
        } else {
            return false;
        }
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
        if (c->has_multi_primary_hash)
        {
            MultiHashTable::ptr indices = make_shared<MultiHashTable>(c->MultiPrimaryFilename(), c->size);
            indices->Delete(record->GetValue(i), rid);
        }
    }
    if (record->td->PrimaryIdxs()->size() > 0u) // 主键
    {
        HashTable::ptr indices = make_shared<HashTable>(record->td->PrimaryFilename(), record->td->PrimarySize());
        indices->Delete(record->PrimaryKey());
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
        if (c->has_multi_primary_hash)
        {
            MultiHashTable::ptr indices = make_shared<MultiHashTable>(c->MultiPrimaryFilename(), c->size);
            indices->Insert(record->GetValue(i), rid);
        }
    }
    if (record->td->PrimaryIdxs()->size() > 0u) // 主键
    {
        HashTable::ptr indices = make_shared<HashTable>(record->td->PrimaryFilename(), record->td->PrimarySize());
        indices->Insert(record->PrimaryKey(), rid);
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
        return data_a != nullptr && data_b != nullptr && compare(cd->typeEnum, data_a, data_b) == 0;
        break;
    case Condition::OP_NEQ:
        return data_a != nullptr && data_b != nullptr && compare(cd->typeEnum, data_a, data_b) != 0;
        break;
    case Condition::OP_LT:
        return data_a != nullptr && data_b != nullptr && compare(cd->typeEnum, data_a, data_b) < 0;
        break;
    case Condition::OP_GT:
        return data_a != nullptr && data_b != nullptr && compare(cd->typeEnum, data_a, data_b) > 0;
        break;
    case Condition::OP_LE:
        return data_a != nullptr && data_b != nullptr && compare(cd->typeEnum, data_a, data_b) <= 0;
        break;
    case Condition::OP_GE:
        return data_a != nullptr && data_b != nullptr && compare(cd->typeEnum, data_a, data_b) >= 0;
        break;
    case Condition::OP_IS_NULL:
        return data_a == nullptr;
        break;
    case Condition::OP_NOT_NULL:
        return data_a != nullptr;
        break;
    case Condition::OP_LIKE:
        {
            assert(cond.expr.expr_type == Expr::EXPR_VALUE);
            assert(cond.expr.value.value_type == Value::VALUE_REGEXP);
            string str((char*)data_a->data(), data_a->size());
            return data_a != nullptr && data_b != nullptr && regex_match(str.begin(), str.end(), cond.expr.value.reg);
        }
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

    BPlusTree::ptr column_indices = nullptr;
    HashTable::ptr primary_hash = nullptr;
    MultiHashTable::ptr multi_primary_hash = nullptr;

    if (cd->indexed)
    {
        column_indices = make_shared<BPlusTree>(cd->IndexFilename(), cd->typeEnum);
    }
    if (cd->is_only_primary)
    {
        primary_hash = make_shared<HashTable>(td->PrimaryFilename(), td->PrimarySize());
    }
    if (cd->has_multi_primary_hash)
    {
        multi_primary_hash = make_shared<MultiHashTable>(cd->MultiPrimaryFilename(), cd->size);
    }
    
    if (cond.expr.expr_type == Expr::EXPR_VALUE)
    {
        auto key = cond.expr.value.data;
        if (cond.op == Condition::OP_EQ)
        {
            if (cd->is_only_primary) return primary_hash->Exists(key);
            if (cd->has_multi_primary_hash) return multi_primary_hash->Count(key);
            if (cd->indexed) return column_indices->EQCount(key);
        } else if (cond.op == Condition::OP_NEQ)
        {
            if (cd->is_only_primary) return primary_hash->TotalRecords() - primary_hash->Exists(key);
            if (cd->has_multi_primary_hash) return multi_primary_hash->TotalRecords() - multi_primary_hash->Count(key);
            if (cd->indexed) return column_indices->TotalCount() - column_indices->EQCount(key);
        } else if (cond.op == Condition::OP_LT)
        {
            if (cd->indexed) return column_indices->LTCount(key);
        } else if (cond.op == Condition::OP_GT)
        {
            if (cd->indexed) return column_indices->TotalCount() - column_indices->LECount(key);
        } else if (cond.op == Condition::OP_LE)
        {
            if (cd->indexed) return column_indices->LECount(key);
        } else if (cond.op == Condition::OP_GE)
        {
            if (cd->indexed) return column_indices->TotalCount() - column_indices->LTCount(key);
        } else if (cond.op == Condition::OP_LIKE)
        {
            // nothing
        } else {
            assert(false);
        }
    }

    return INF;
}

vector<int> list_condition_rids(TableDesc::ptr td, const Condition& cond)
{
    ColDesc::ptr cd = td->Column(cond.column.col_name);

    BPlusTree::ptr column_indices = nullptr;
    HashTable::ptr primary_hash = nullptr;
    MultiHashTable::ptr multi_primary_hash = nullptr;

    if (cd->indexed)
    {
        column_indices = make_shared<BPlusTree>(cd->IndexFilename(), cd->typeEnum);
    }
    if (cd->is_only_primary)
    {
        primary_hash = make_shared<HashTable>(td->PrimaryFilename(), td->PrimarySize());
    }
    if (cd->has_multi_primary_hash)
    {
        multi_primary_hash = make_shared<MultiHashTable>(cd->MultiPrimaryFilename(), cd->size);
    }

    if (cond.expr.expr_type == Expr::EXPR_VALUE)
    {
        auto key = cond.expr.value.data;
        if (cond.op == Condition::OP_EQ)
        {
            if (cd->is_only_primary)
            {
                vector<int> ans;
                if (primary_hash->Exists(key)) ans.push_back(primary_hash->Fetch(key));
                return ans;
            }
            if (cd->has_multi_primary_hash)
            {
                if (multi_primary_hash->Count(key) > 0)
                    return *(multi_primary_hash->Fetch(key).get());
            }
            if (cd->indexed)
            {
                vector<int> ans;
                auto iter = column_indices->Lower(key);
                while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) == 0)
                {
                    ans.push_back(iter.Value());
                    iter.Next();
                }
                return ans;
            }
        } else if (cond.op == Condition::OP_NEQ)
        {
            if (cd->indexed)
            {
                vector<int> ans;
                auto iter = column_indices->Begin();
                while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) < 0)
                {
                    ans.push_back(iter.Value());
                    iter.Next();
                }
                iter = column_indices->Upper(key);
                while(!iter.End())
                {
                    ans.push_back(iter.Value());
                    iter.Next();
                }
                return ans;
            }
        } else if (cond.op == Condition::OP_LT)
        {
            if (cd->indexed)
            {
                vector<int> ans;
                auto iter = column_indices->Begin();
                while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) < 0)
                {
                    ans.push_back(iter.Value());
                    iter.Next();
                }
                return ans;
            }
        } else if (cond.op == Condition::OP_GT)
        {
            if (cd->indexed)
            {
                vector<int> ans;
                auto iter = column_indices->Upper(key);
                while(!iter.End())
                {
                    ans.push_back(iter.Value());
                    iter.Next();
                }
                return ans;
            }
        } else if (cond.op == Condition::OP_LE)
        {
            if (cd->indexed)
            {
                vector<int> ans;
                auto iter = column_indices->Begin();
                while(!iter.End() && compare(cd->typeEnum, iter.Key(), key) <= 0)
                {
                    ans.push_back(iter.Value());
                    iter.Next();
                }
                return ans;
            }
        } else if (cond.op == Condition::OP_GE)
        {
            if (cd->indexed)
            {
                vector<int> ans;
                auto iter = column_indices->Lower(key);
                while(!iter.End())
                {
                    ans.push_back(iter.Value());
                    iter.Next();
                }
                return ans;
            }
        } else if (cond.op == Condition::OP_LIKE)
        {
            // nothing
        } else {
            assert(false);
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
    assert((int)key->size() == td->PrimarySize());
    HashTable::ptr indices = make_shared<HashTable>(td->PrimaryFilename(), td->PrimarySize());
    return indices->Exists(key) ? indices->Fetch(key) : -1;
}

vector<int> search_in_oneof_primary(ColDesc::ptr cd, data_t key)
{
    vector<int> ans;
    if (cd->is_only_primary)
    {
        HashTable::ptr indices = make_shared<HashTable>(cd->td->PrimaryFilename(), cd->td->PrimarySize());
        if (indices->Exists(key))
        {
            ans.push_back(indices->Fetch(key));
        }
    } else if (cd->has_multi_primary_hash){
        MultiHashTable::ptr indices = make_shared<MultiHashTable>(cd->MultiPrimaryFilename(), cd->size);
        if (indices->Count(key) > 0)
        {
            ans = *indices->Fetch(key).get();
        }
    } else {
        assert(false);
    }
    return ans;
}

void delete_records(TableDesc::ptr td, const vector<int>& rids)
{
    SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
    for(auto rid : rids)
    {
        Record::ptr record = td->RecoverRecord(file->Fetch(rid));

        for(int i = 0; i < (int)record->td->cols.size(); i ++)
        {
            auto cd = record->td->cols[i];
            if (cd->is_only_primary && cd->be_refed_tbs.size() > 0u)
            {
                for(int j = 0; j < (int)cd->be_refed_tbs.size(); j ++)
                {
                    auto be_refed_td = td->dd->SearchTable(cd->be_refed_tbs[j]);
                    auto be_refed_cd = be_refed_td->Column(cd->be_refed_col_idx[j]);
                    vector<int> be_refed_rids = search_in_oneof_primary(be_refed_cd, record->GetValue(i));

                    delete_records(be_refed_td, be_refed_rids);
                }
            }
        }

        file->Delete(rid);
        remove_record_from_indices(record, rid);
    }
}