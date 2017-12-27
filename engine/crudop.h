// 增删查该
#ifndef ENGINE_CRUD_OP_H
#define ENGINE_CRUD_OP_H

#include "context.h"
#include <disk/common.h>
#include <cstdio>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>

using namespace std;

// 值
class Value
{
public:
    enum ValueType { VALUE_INT, VALUE_STRING, VALUE_FLOAT, VALUE_NULL };
    ValueType value_type;
    data_t data;

    static Value int_value(int value) { Value ans; ans.value_type = VALUE_INT; ans.data = int_data(value); return ans; }
    static Value float_value(float value) { Value ans; ans.value_type = VALUE_FLOAT; ans.data = float_data(value); return ans; }
    static Value string_value(const string& value) { Value ans; ans.value_type = VALUE_STRING; ans.data = string_data(value); return ans; }
    static Value null_value() { Value ans; ans.value_type = VALUE_NULL; ans.data = nullptr; return ans; }
    
    void string_to_date()
    {
        std::tm tm = {};
        std::stringstream ss(string((char*)data->data(), data->size()));
        ss >> std::get_time(&tm, "%Y-%m-%d");
        time_t t = std::mktime(&tm);

        data = alloc_data(0);
        append(data, t);
    }
    void int_to_float()
    {
        int value = *(int*)data->data();
        *(float*)data->data() = value;
        value_type = VALUE_FLOAT;
    }

    string stringify() const
    {
        char buf[1024];
        switch(value_type)
        {
        case VALUE_INT:
            sprintf(buf, "%d", *(int*)(data->data()));
            break;
        case VALUE_STRING:
            sprintf(buf, "'%s'", string((char*)data->data(), data->size()).c_str());
            break;
        case VALUE_FLOAT:
            sprintf(buf, "%f", *(float*)(data->data()));
            break;
        case VALUE_NULL:
            return "NULL";
        }
        return buf;
    }
};
// 列
class Column
{
public:
    bool col_name_only;
    string tb_name;
    string col_name;

    static Column create_column(const string& col_name) { Column ans; ans.col_name_only = true; ans.col_name = col_name; return ans; }
    static Column create_column(const string& tb_name, const string& col_name) { Column ans; ans.col_name_only = false; ans.tb_name = tb_name; ans.col_name = col_name; return ans; }
};
// 表达式
class Expr
{
public:
    enum ExprType { EXPR_COLUMN, EXPR_VALUE };
    ExprType expr_type;
    Column column;
    Value value;

    static Expr column_expr(const Column& column) { Expr ans; ans.expr_type = EXPR_COLUMN; ans.column = column; return ans; }
    static Expr value_expr(const Value& value) { Expr ans; ans.expr_type = EXPR_VALUE; ans.value = value; return ans; }
};
// 条件
class Condition
{
public:
    enum OP { OP_EQ, OP_NEQ, OP_LE, OP_GE, OP_LT, OP_GT, OP_IS_NULL, OP_NOT_NULL };
    Column column;
    OP op;
    Expr expr;

    static Condition expr_condition(const Column& column, OP op, const Expr& expr) { Condition ans; ans.column = column; ans.op = op; ans.expr = expr; return ans; }
    static Condition is_null_condition(const Column& column) { Condition ans; ans.column = column; ans.op = OP_IS_NULL; return ans; }
    static Condition not_null_condition(const Column& column) { Condition ans; ans.column = column; ans.op = OP_NOT_NULL; return ans; }

    void reverse_op()
    {
        switch(op)
        {
        case OP_EQ: case OP_NEQ: break;
        case OP_LE: op = OP_GE; break;
        case OP_LT: op = OP_GT; break;
        case OP_GE: op = OP_LE; break;
        case OP_GT: op = OP_LT; break;
        case OP_IS_NULL: case OP_NOT_NULL: assert(false);
        }
    }

    bool is_binary_operator() const
    {
        return op != OP_IS_NULL && op != OP_NOT_NULL;
    }
};
// 设值
class Assignment
{
public:
    string column;
    Value value;

    static Assignment assign(const string& column, const Value& value) { Assignment ans; ans.column = column; ans.value = value; return ans; }
};
// 索检
class Selector
{
public:
    enum SelectorType { SELECT_ALL, SELECT_COLUMNS };
    SelectorType selector_type;
    vector<Column> columns;

    static Selector all_selector() { Selector ans; ans.selector_type = SELECT_ALL; return ans; }
    static Selector columns_selector(const vector<Column>& columns) { Selector ans; ans.selector_type = SELECT_COLUMNS; ans.columns = columns; return ans; }
};

void insert_op(Context* ctx, const string& tb_name, vector<vector<Value> > values_list);
void delete_op(Context* ctx, const string& tb_name, vector<Condition> conditions);
void update_op(Context* ctx, const string& tb_name, vector<Assignment> assignments, vector<Condition> conditions);
void select_op(Context* ctx, Selector selector, vector<string> tables, vector<Condition> conditions);

#endif // ENGINE_CRUD_OP_H