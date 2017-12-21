#ifndef FRONTEND_STMTS_TB_H
#define FRONTEND_STMTS_TB_H

#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "stmts_base.hpp"

using namespace std;

class CreateTable : public Statement
{
public:
    class Field
    {
    public:
        int type_no; // 1~4
        ColumnDefine column_define;
        string primary_column;
        ColumnForeign foreign_column;

        Field(){}
    };

    string tb_name;
    vector<Field> fields;

    CreateTable(const string& tb_name, const vector<Field>& fields): tb_name(tb_name), fields(fields) {}

    void run(Context* ctx)
    {
        vector<ColumnDefine> cols;
        vector<string> primary_cols;
        vector<ColumnForeign> foreign_cols;

        for(auto& x : fields)
        {
            if (x.type_no == 1 || x.type_no == 2)
            {
                cols.push_back(x.column_define);
            } else if (x.type_no == 3)
            {
                primary_cols.push_back(x.primary_column);
            } else if (x.type_no == 4)
            {
                foreign_cols.push_back(x.foreign_column);
            } else {
                assert(false);
            }
        }

        create_table(ctx, tb_name, cols, primary_cols, foreign_cols);
    }
};

class DropTable : public Statement
{
public:
    string tb_name;

    DropTable(const string& tb_name): tb_name(tb_name) {}

    void run(Context* ctx)
    {
        drop_table(ctx, tb_name);
    }
};

class DescTable : public Statement
{
public:
    string tb_name;

    DescTable(const string& tb_name): tb_name(tb_name) {}

    void run(Context* ctx)
    {
        desc_table(ctx, tb_name);
    }
};

class InsertStatement : public Statement
{
public:
    string tb_name;
    vector<vector<Value>> values_lists;

    InsertStatement(const string& tb_name, const vector<vector<Value>>& values_lists): tb_name(tb_name), values_lists(values_lists) {}

    void run(Context* ctx)
    {
        insert_op(ctx, tb_name, values_lists);
    }
};

class DeleteStatement : public Statement
{
public:
    string tb_name;
    vector<Condition> conditions;
    
    DeleteStatement(const string& tb_name, const vector<Condition>& conditions): tb_name(tb_name), conditions(conditions) {}

    void run(Context* ctx)
    {
        delete_op(ctx, tb_name, conditions);
    }
};

class UpdateStatement : public Statement
{
public:
    string tb_name;
    vector<Assignment> assignments;
    vector<Condition> conditions;

    UpdateStatement(const string& tb_name, const vector<Assignment>& assignments, const vector<Condition>& conditions): tb_name(tb_name), assignments(assignments), conditions(conditions) {}

    void run(Context* ctx)
    {
        update_op(ctx, tb_name, assignments, conditions);
    }
};

class SelectStatement : public Statement
{
public:
    Selector selector;
    vector<string> tables;
    vector<Condition> conditions;

    SelectStatement(const Selector& selector, const vector<string>& tables, const vector<Condition>& conditions): selector(selector), tables(tables), conditions(conditions) {}

    void run(Context* ctx)
    {
        select_op(ctx, selector, tables, conditions);
    }
};

#endif // FRONTEND_STMTS_TB_H