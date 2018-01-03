// 增删查该
#ifndef ENGINE_CRUD_OP_H
#define ENGINE_CRUD_OP_H

#include <json11.hpp>
#include "context.h"
#include <disk/common.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <regex>
#include <algorithm>

using namespace std;

// 值
class Value
{
public:
    enum ValueType { VALUE_INT, VALUE_STRING, VALUE_FLOAT, VALUE_NULL,
        VALUE_DECIMAL /*定点数，只能转换*/,
        VALUE_REGEXP /*正则表达式，只能转换*/
    };
    ValueType value_type;
    data_t data;
    string origin_value;
    regex reg;

    static Value int_value(const string& value) {
        Value ans;
        ans.value_type = VALUE_INT;
        ans.data = int_data(atoi(value.c_str()));
        ans.origin_value = value;
        return ans;
    }
    static Value float_value(const string& value) {
        Value ans;
        ans.value_type = VALUE_FLOAT;
        ans.data = float_data(atof(value.c_str()));
        ans.origin_value = value;
        return ans;
    }
    static Value string_value(const string& value) {
        Value ans;
        ans.value_type = VALUE_STRING;
        ans.data = string_data(value);
        ans.origin_value = value;
        return ans;
    }
    static Value null_value() {
        Value ans;
        ans.value_type = VALUE_NULL;
        ans.data = nullptr;
        return ans;
    }
    Json basic_to_json() {
        switch(value_type)
        {
        case VALUE_INT:
            return Json(*(int*)data->data());
        case VALUE_STRING:
            return Json(string((char*)data->data(), data->size()));
        case VALUE_FLOAT:
            return Json(*(float*)data->data());
        default: assert(false);
        }
    }
    
    bool string_to_date()
    {
        { // check
            const static int MAX_DAYS[] = {
                0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
            };
            string str((char*)data->data(), data->size());
            int year, mon, day;
            if (sscanf(str.c_str(), "%d-%d-%d", &year, &mon, &day) != 3) return false;
            if (year < 1970 || 3000 < year) return false;
            if (mon < 1 || 12 < mon) return false;
            if (day < 1 || 31 < day) return false;
            bool run = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
            if (day > MAX_DAYS[mon] + int(run&&(mon==2))) return false;
        }

        std::tm tm = {};
        std::stringstream ss(string((char*)data->data(), data->size()));
        ss >> std::get_time(&tm, "%Y-%m-%d");
        time_t t = std::mktime(&tm);

        data = alloc_data(0);
        append(data, t);

        return true;
    }
    void int_to_float()
    {
        int value = *(int*)data->data();
        *(float*)data->data() = value;
        value_type = VALUE_FLOAT;
    }
    void float_to_decimal()
    {
        char bbuf[20] = {0};
        int a, b;
        sscanf(origin_value.c_str(), "%d.%s", &a, bbuf);
        while(strlen(bbuf) < 9) bbuf[strlen(bbuf)] = '0';
        b = atoi(bbuf);
        data = int_data(a);
        append(data, b);
        value_type = VALUE_DECIMAL;
    }
    void int_to_decimal()
    {
        int a = atoi(origin_value.c_str()), b = 0;
        data = int_data(a);
        append(data, b);
        value_type = VALUE_DECIMAL;
    }
    void string_to_regexp()
    {
        std::regex specialChars{ R"([-[\]{}()*+?.,\^$|#\s])" };
        std::string sanitized = std::regex_replace(origin_value, specialChars, R"(\$&)");
        size_t pos;
        while((pos = sanitized.find("%")) != string::npos) sanitized.replace(pos, 1, ".*");
        while((pos = sanitized.find("_")) != string::npos) sanitized.replace(pos, 1, ".");
        reg = regex("^" + sanitized + "$");
        value_type = VALUE_REGEXP;
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
        case VALUE_DECIMAL:
            sprintf(buf, "%d.%d", *(int*)(data->data()), *(int*)(data->data()+sizeof(int)));
            break;
        case VALUE_REGEXP:
            return origin_value;
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
    enum OP { OP_EQ, OP_NEQ, OP_LE, OP_GE, OP_LT, OP_GT, OP_IS_NULL, OP_NOT_NULL, OP_LIKE };
    Column column;
    OP op;
    Expr expr;

    static Condition expr_condition(const Column& column, OP op, const Expr& expr) { Condition ans; ans.column = column; ans.op = op; ans.expr = expr; return ans; }
    static Condition like_condition(const Column& column, const string& pattern) { Condition ans; ans.column = column; ans.op = OP_LIKE; ans.expr = Expr::value_expr(Value::string_value(pattern)); return ans; }
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
        case OP_IS_NULL: case OP_NOT_NULL: case OP_LIKE: assert(false);
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
    enum SelectorType { SELECT_ALL, SELECT_COLUMNS, SELECT_AVG, SELECT_SUM, SELECT_MIN, SELECT_MAX, SELECT_COUNT };
    SelectorType selector_type; // 查询类型
    vector<Column> columns; // 查询的列
    bool is_aggregate_query; // 是否是聚合类型
    bool has_query_column;
    Column query_column; // 聚合类型的查询列

    static Selector all_selector() {
        Selector ans;
        ans.selector_type = SELECT_ALL;
        ans.is_aggregate_query = false;
        ans.has_query_column = false;
        return ans;
    }
    static Selector columns_selector(const vector<Column>& columns) {
        Selector ans;
        ans.selector_type = SELECT_COLUMNS;
        ans.columns = columns;
        ans.is_aggregate_query = false;
        ans.has_query_column = false;
        return ans;
    }
    static Selector avg_selector(const Column& column) {
        Selector ans;
        ans.selector_type = SELECT_AVG;
        ans.query_column = column;
        ans.is_aggregate_query = true;
        ans.has_query_column = true;
        return ans;
    }
    static Selector sum_selector(const Column& column) {
        Selector ans;
        ans.selector_type = SELECT_SUM;
        ans.query_column = column;
        ans.is_aggregate_query = true;
        ans.has_query_column = true;
        return ans;
    }
    static Selector min_selector(const Column& column) {
        Selector ans;
        ans.selector_type = SELECT_MIN;
        ans.query_column = column;
        ans.is_aggregate_query = true;
        ans.has_query_column = true;
        return ans;
    }
    static Selector max_selector(const Column& column) {
        Selector ans;
        ans.selector_type = SELECT_MAX;
        ans.query_column = column;
        ans.is_aggregate_query = true;
        ans.has_query_column = true;
        return ans;
    }
    static Selector count_selector() {
        Selector ans;
        ans.selector_type = SELECT_COUNT;
        ans.is_aggregate_query = true;
        ans.has_query_column = false;
        return ans;
    }
};

void insert_op(Context* ctx, const string& tb_name, vector<vector<Value> > values_list);
void delete_op(Context* ctx, const string& tb_name, vector<Condition> conditions);
void update_op(Context* ctx, const string& tb_name, vector<Assignment> assignments, vector<Condition> conditions);
void select_op(Context* ctx, Selector selector, vector<string> tables, vector<Condition> conditions);

#endif // ENGINE_CRUD_OP_H