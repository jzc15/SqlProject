#include "crudop.h"
#include "helper.h"
#include "topsort.h"
#include <disk/common.h>
#include <datamanager/slotsfile.h>
#include <cassert>
#include <iostream>
#include <functional>

using namespace std;

void insert_op(Context* ctx, const string& tb_name, vector<vector<Value> > values_list)
{
    TITLE(insert_op)

    auto td = ctx->dd->SearchTable(tb_name);
    SlotsFile::ptr table_file = make_shared<SlotsFile>(td->disk_filename);

    for(auto& values : values_list)
    {
        Record::ptr record = td->NewRecord();
        if (values.size() != td->cols.size())
        {
            cerr << "ERROR AT INSERT : wrong num of values" << endl;
            return;
        }

        for(int i = 0; i < (int)values.size(); i ++)
        {
            auto cd = td->Column(i);
            Value& v = values[i];
            if (!value_type_trans_ok(cd->typeEnum, v))
            {
                cerr << "ERROR AT INSERT : wrong type for column `" << cd->columnName << "` : " << v.stringify() << endl;
                return;
            }
            record->SetValue(i, v.data);
        }
        if (!record_check_ok(record))
        {
            cerr << "ERROR AT INSERT : wrong record" << endl;
            return;
        }
        int rid = table_file->Insert(record->Generate());
        insert_record_to_indices(record, rid);
    }
}
void delete_op(Context* ctx, const string& tb_name, vector<Condition> conditions)
{
    TITLE(delete_op);
    auto td = ctx->dd->SearchTable(tb_name);
    for(auto& cond : conditions)
    {
        if (cond.is_binary_operator())
        {
            auto cd_a = td->Column(cond.column.col_name);
            if (cond.expr.expr_type == Expr::EXPR_COLUMN && cd_a->typeEnum != td->Column(cond.expr.column.col_name)->typeEnum)
            {
                cerr << "ERROR AT DELETE : wrong type between column `" << cd_a->columnName << "` and column `" <<  td->Column(cond.expr.column.col_name)->columnName << "`" << endl;
                return;
            }
            if (cond.expr.expr_type == Expr::EXPR_VALUE && !value_type_trans_ok(cd_a->typeEnum, cond.expr.value))
            {
                cerr << "ERROR AT DELETE : wrong type for column `" << cd_a->columnName << "` : " << cond.expr.value.stringify() << endl;
                return;
            }
        }
    }

    vector<int> rids = list_conditions_rids(td, conditions);
    SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
    for(auto rid : rids)
    {
        Record::ptr record = td->RecoverRecord(file->Fetch(rid));
        file->Delete(rid);
        remove_record_from_indices(record, rid);
    }
    cout << "deleted count : " << rids.size() << endl;
}
void update_op(Context* ctx, const string& tb_name, vector<Assignment> assignments, vector<Condition> conditions)
{
    TITLE(update_op);
    auto td = ctx->dd->SearchTable(tb_name);
    for(auto& cond : conditions)
    {
        if (cond.is_binary_operator())
        {
            auto cd_a = td->Column(cond.column.col_name);
            if (cond.expr.expr_type == Expr::EXPR_COLUMN && cd_a->typeEnum != td->Column(cond.expr.column.col_name)->typeEnum)
            {
                cerr << "ERROR AT UPDATE CONDITION : wrong type between column `" << cd_a->columnName << "` and column `" <<  td->Column(cond.expr.column.col_name)->columnName << "`" << endl;
                return;
            }
            if (cond.expr.expr_type == Expr::EXPR_VALUE && !value_type_trans_ok(cd_a->typeEnum, cond.expr.value))
            {
                cerr << "ERROR AT UPDATE CONDITION : wrong type for column `" << cd_a->columnName << "` : " << cond.expr.value.stringify() << endl;
                return;
            }
        }
    }
    for(auto& assign: assignments)
    {
        auto cd = td->Column(assign.column);
        if (!value_type_trans_ok(cd->typeEnum, assign.value))
        {
            cerr << "ERROR AT UPDATE SET : wrong type for column `" << cd->columnName << "` : " << assign.value.stringify() << endl;
            return;
        }
    }

    vector<int> rids = list_conditions_rids(td, conditions);
    SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
    for(const auto rid : rids)
    {
        Record::ptr old_record = td->RecoverRecord(file->Fetch(rid));
        Record::ptr new_record = Record::Clone(old_record);
        for(const auto& assign : assignments)
        {
            new_record->SetValue(assign.column, assign.value.data);
        }
        if (record_check_ok(new_record, rid))
        {
            file->Delete(rid);
            remove_record_from_indices(old_record, rid);
            int nrid = file->Insert(new_record->Generate());
            insert_record_to_indices(new_record, nrid);
        } else {
            cerr << "ERROR AT INSERT : wrong record" << endl;
            return;
        }
    }
    cout << "updated count : " << rids.size() << endl;
}
void select_op(Context* ctx, Selector selector, vector<string> tables, vector<Condition> conditions)
{
    TITLE(select_op);

    for(int i = 0; i < (int)tables.size(); i ++)
        for(int j = i+1; j < (int)tables.size(); j ++)
            if (tables[i] == tables[j])
            {
                cerr << "ERROR AT SELECT : same table name `" << tables[i] << "`" << endl;
                return;
            }

    // slove conditions' Column tb_name
    for(auto& cond: conditions)
    {
        solve_column_tb_name(ctx, tables, cond.column);
        if (cond.is_binary_operator() && cond.expr.expr_type == Expr::EXPR_COLUMN)
        {
            solve_column_tb_name(ctx, tables, cond.expr.column);
        }
    }
    if (selector.selector_type == Selector::SELECT_COLUMNS)
    {
        for(auto& ccolumn : selector.columns)
        {
            solve_column_tb_name(ctx, tables, ccolumn);
        }
    }

    // type check
    for(auto& cond : conditions)
    {
        if (cond.is_binary_operator())
        {
            auto cd_a = ctx->dd->SearchTable(cond.column.tb_name)->Column(cond.column.col_name);
            if (cond.expr.expr_type == Expr::EXPR_COLUMN && cd_a->typeEnum != ctx->dd->SearchTable(cond.expr.column.tb_name)->Column(cond.expr.column.col_name)->typeEnum)
            {
                cerr << "ERROR AT UPDATE CONDITION : wrong type between column `" << cd_a->columnName << "` and column `" <<  ctx->dd->SearchTable(cond.expr.column.tb_name)->Column(cond.expr.column.col_name)->columnName << "`" << endl;
                return;
            }
            if (cond.expr.expr_type == Expr::EXPR_VALUE && !value_type_trans_ok(cd_a->typeEnum, cond.expr.value))
            {
                cerr << "ERROR AT UPDATE CONDITION : wrong type for column `" << cd_a->columnName << "` : " << cond.expr.value.stringify() << endl;
                return;
            }
        }
    }

    // sort tables
    {
        set<string> in_tables;
        vector<string> new_tables;

        // 主键优化
        for(const auto& cond : conditions)
        {
            if (in_tables.find(cond.column.tb_name) != in_tables.end()) continue;
            if (cond.is_binary_operator()
                && cond.expr.expr_type == Expr::EXPR_VALUE && cond.op == Condition::OP_EQ
                && ctx->dd->SearchTable(cond.column.tb_name)->Column(cond.column.col_name)->is_only_primary) {
                    in_tables.insert(cond.column.tb_name);
                    new_tables.push_back(cond.column.tb_name);
                }
        }

        // 主键成员优化
        for(const auto& cond : conditions)
        {
            if (in_tables.find(cond.column.tb_name) != in_tables.end()) continue;
            if (cond.is_binary_operator()
                && cond.expr.expr_type == Expr::EXPR_VALUE && cond.op == Condition::OP_EQ
                && ctx->dd->SearchTable(cond.column.tb_name)->Column(cond.column.col_name)->is_oneof_primary) {
                    in_tables.insert(cond.column.tb_name);
                    new_tables.push_back(cond.column.tb_name);
                }
        }

        TopSort<string> top_sort;
        for(const auto& tb : tables)
        {
            if (in_tables.find(tb) != in_tables.end()) continue;
            top_sort.TouchNode(tb);
        }
        for(const auto& cond : conditions)
        {
            if (cond.is_binary_operator() && cond.expr.expr_type == Expr::EXPR_COLUMN && cond.op == Condition::OP_EQ)
            {
                const Column& a = cond.column;
                const Column& b = cond.expr.column;
                if (in_tables.find(a.tb_name) != in_tables.end()) continue;
                if (in_tables.find(b.tb_name) != in_tables.end()) continue;
                if (a.tb_name != b.tb_name)
                {
                    auto td_a = ctx->dd->SearchTable(a.tb_name);
                    auto td_b = ctx->dd->SearchTable(b.tb_name);
                    auto cd_a = td_a->Column(a.col_name);
                    auto cd_b = td_b->Column(b.col_name);
                    if (cd_a->is_only_primary)
                    {
                        top_sort.Build(b.tb_name, a.tb_name);
                    } else if (cd_b->is_only_primary)
                    {
                        top_sort.Build(a.tb_name, b.tb_name);
                    }
                }
            }
        }
        auto top_que = top_sort.Sort();
        assert(top_que != nullptr);
        for(const auto& tb : *top_que.get())
        {
            new_tables.push_back(tb);
        }

        tables = new_tables;
        cout << "table list : ";
        for(const auto& tb : tables)
        {
            cout << tb << " ";
        }
        cout << endl;
    }
    map<string, int> tables_idx;
    for(int i = 0; i < (int)tables.size(); i ++) tables_idx[tables[i]] = i;

    // select
    vector<vector<int>> table_records(tables.size());
    vector<TableDesc::ptr> tds(tables.size());
    // 是否被前面的表所引用
    vector<int> be_refed_tb_idx(tables.size(), -1), be_refed_col_idx(tables.size(), -1), be_refed_me_col_idx(tables.size(), -1);
    for(int i = 0; i < (int)tables.size(); i ++) tds[i] = ctx->dd->SearchTable(tables[i]);
    // 对于跨越两个表的条件，交换为前面和后面的比较
    for(auto& cond : conditions)
    {
        if (cond.is_binary_operator() && cond.expr.expr_type == Expr::EXPR_COLUMN)
        {
            Column& a = cond.column;
            Column& b = cond.expr.column;
            if (tables_idx[a.tb_name] > tables_idx[b.tb_name])
            {
                swap(a, b);
                cond.reverse_op();
            }
        }
    }
    for(int i = 0; i < (int)tables.size(); i ++)
    {
        be_refed_tb_idx[i] = -1;
        for(const auto& cond : conditions)
        {
            if (cond.is_binary_operator() && cond.expr.expr_type == Expr::EXPR_COLUMN && cond.op == Condition::OP_EQ)
            {
                const Column& a = cond.column;
                const Column& b = cond.expr.column;
                if (b.tb_name == tables[i] && tds[i]->Column(b.col_name)->is_oneof_primary && tables_idx[a.tb_name] < i)
                {
                    be_refed_tb_idx[i] = tables_idx[a.tb_name];
                    be_refed_col_idx[i] = tds[tables_idx[a.tb_name]]->ColumnIndex(a.col_name);
                    be_refed_me_col_idx[i] = tds[i]->ColumnIndex(b.col_name);
                    break;
                }
            }
        }
    }
    for(int i = 0; i < (int)tables.size(); i ++)
        if (be_refed_tb_idx[i] == -1)
        {
            vector<Condition> this_conds;
            for(const auto& cond : conditions)
            {
                const Column& c = cond.column;
                if (c.tb_name == tables[i] && (!cond.is_binary_operator() || cond.expr.expr_type != Expr::EXPR_COLUMN || cond.expr.column.tb_name == tables[i]))
                {
                    this_conds.push_back(cond);
                }
            }
            table_records[i] = list_conditions_rids(tds[i], this_conds);
            cout << "plain records : " << tables[i] << " conds size = " << this_conds.size() << " records size = " << table_records[i].size() << endl;
        }
    
    // prepare output titles
    vector<string> output_titles;
    vector<int> output_record_idx;
    vector<int> output_record_col_idx;
    vector<type_t> output_types;
    if (selector.selector_type == Selector::SELECT_ALL)
    {
        for(int i = 0; i < (int)tds.size(); i ++)
        {
            auto td = tds[i];
            for(int j = 0; j < (int)td->cols.size(); j ++)
            {
                auto cd = td->cols[j];
                output_titles.push_back(td->tableName + "." + cd->columnName);
                output_record_idx.push_back(i);
                output_record_col_idx.push_back(j);
                output_types.push_back(cd->typeEnum);
            }
        }
    } else {
        for(const auto& c : selector.columns)
        {
            int i = tables_idx[c.tb_name];
            auto td = tds[i];
            int j = td->ColumnIndex(c.col_name);
            auto cd = td->cols[j];
            output_titles.push_back(td->tableName + "." + cd->columnName);
            output_record_idx.push_back(i);
            output_record_col_idx.push_back(j);
            output_types.push_back(cd->typeEnum);
        }
    }

    // search records pairs
    vector<Record::ptr> current_records(tables.size()); // 当前每个表的记录
    vector<SlotsFile::ptr> files(tables.size()); // 每个表的磁盘文件
    for(int i = 0; i < (int)tables.size(); i ++) files[i] = make_shared<SlotsFile>(tds[i]->disk_filename);
    int selected_count = 0;
    std::function<void(int)> select;
    select = [&](int table_idx) {
        if (table_idx >= (int)tables.size())
        {
            // new record
            if (selected_count % 10 == 0)
            {
                cout << endl;
                for(int i = 0; i < (int)output_titles.size(); i ++)
                {
                    if (i) cout << " : ";
                    cout << output_titles[i];
                }
                cout << endl;
            }
            for(int i = 0; i < (int)output_titles.size(); i ++)
            {
                if (i) cout << " : ";
                cout << stringify(output_types[i], current_records[output_record_idx[i]]->GetValue(output_record_col_idx[i]));
            }
            cout << endl;
            selected_count ++;
            return;
        }
        if (be_refed_tb_idx[table_idx] != -1) // 被引用
        {
            data_t refed_key = current_records[be_refed_tb_idx[table_idx]]->GetValue(be_refed_col_idx[table_idx]);
            if (refed_key == nullptr) return;

            vector<int> this_rids = search_in_oneof_primary(tds[table_idx]->Column(be_refed_me_col_idx[table_idx]), refed_key);
            for(int rid : this_rids)
            {
                Record::ptr record = current_records[table_idx] = tds[table_idx]->RecoverRecord(files[table_idx]->Fetch(rid));
                bool flag = true;
                for(const auto& cond: conditions)
                {
                    const Column& c = cond.column;
                    if (c.tb_name == tables[table_idx])
                    {
                        if(!cond.is_binary_operator() || cond.expr.expr_type != Expr::EXPR_COLUMN || cond.expr.column.tb_name == tables[table_idx])
                        {
                            flag &= test_condition(record, cond);
                        }
                    } else if (cond.is_binary_operator() && cond.expr.expr_type == Expr::EXPR_COLUMN && cond.expr.column.tb_name == tables[table_idx]) {
                        flag &= test_condition(current_records[tables_idx[cond.column.tb_name]], record, cond);
                    }
                    if (!flag) break;
                }
                if (!flag) continue;
                select(table_idx + 1);
            }
        } else {
            for(auto rid : table_records[table_idx])
            {
                Record::ptr record = current_records[table_idx] = tds[table_idx]->RecoverRecord(files[table_idx]->Fetch(rid));
                bool flag = true;
                for(const auto& cond: conditions)
                {
                    if (cond.is_binary_operator() && cond.expr.expr_type == Expr::EXPR_COLUMN && cond.expr.column.tb_name == tables[table_idx]) {
                        flag &= test_condition(current_records[tables_idx[cond.column.tb_name]], record, cond);
                    }
                    if (!flag) break;
                }
                if (!flag) continue;
                select(table_idx + 1);
            }
        }
    };
    select(0);
    cout << "selected count : " << selected_count << endl;
}
