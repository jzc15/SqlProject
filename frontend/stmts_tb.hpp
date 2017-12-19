#ifndef FRONTEND_STMTS_TB_H
#define FRONTEND_STMTS_TB_H

#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "context.hpp"
#include "stmts_base.hpp"

using namespace std;

class TypeExpr
{
public:
    Type type;
    int limit;

    TypeExpr(Type type, int limit = 1): type(type), limit(limit) {}
    string typeName()
    {
        return type_name(type);
    }
};

class Field // 定义列/描述列
{
public:
    bool new_col;

    string* colName;
    TypeExpr* type;
    bool not_null;

    bool is_key;
    string* ref_tbName;
    string* ref_colName;

    Field(string* colName, TypeExpr* type, bool not_null = false)
        : new_col(true), colName(colName), type(type), not_null(not_null)
        {

        }
    Field(string* colName, bool is_key, string* ref_tbName, string* ref_colName)
        : new_col(false), colName(colName), is_key(is_key), ref_tbName(ref_tbName), ref_colName(ref_colName)
        {

        }
};

class FieldList
{
public:
    vector<Field*> fields;
};

class CreateTable : public Statement
{
public:
    string* tbName;
    FieldList* fieldList;

    CreateTable(string* tbName, FieldList* fieldList): tbName(tbName), fieldList(fieldList) {}

    void run(Context* ctx)
    {
        TableDesc::ptr td = ctx->dd->CreateTable(*tbName);
        for(auto field: fieldList->fields)
        {
            if (field->new_col)
            {
                td->CreateColumn(*(field->colName), field->type->typeName(), (field->type->typeName() == "varchar") * field->type->limit); // FIXME
            }
        }
        td->Finalize();
        mkfile(td->disk_filename);
    }
};

class DropTable : public Statement
{
public:
    string* tbName;

    DropTable(string* tbName): tbName(tbName) {}

    void run(Context* ctx)
    {
        TableDesc::ptr td = ctx->dd->DropTable(*tbName);
        rmfile(td->disk_filename);
    }
};

class DescTable : public Statement
{
public:
    string* tbName;

    DescTable(string* tbName): tbName(tbName) {}

    void run(Context* ctx)
    {
        TableDesc::ptr td = ctx->dd->SearchTable(*tbName);
        cout << "========= TABLE [" << *tbName << "] DESC ========" << endl;
        for(int i = 0; i < (int)td->cols.size(); i ++)
        {
            cout << td->cols[i]->columnName << " " << td->cols[i]->typeName << endl;
        }
    }
};

class Value
{
public:
    enum value_t { VALUE_INT, VALUE_STRING, VALUE_NULL };
    value_t value_type;
    int int_value;
    string* string_value;

    Value(): value_type(VALUE_NULL) {}
    Value(int int_value): value_type(VALUE_INT), int_value(int_value) {}
    Value(string* string_value): value_type(VALUE_STRING), string_value(string_value) {}

    void SetValue(Record::ptr record, ColDesc::ptr col)
    {
        if (value_type == VALUE_NULL)
        {
            record->SetNull(col->columnName);
            return;
        }
        if (col->typeName == INT_TYPE)
        {
            assert(value_type == VALUE_INT);
            record->SetInt(col->columnName, int_value);
        } else if (col->typeName == CHAR_TYPE && col->length == 1) {
            assert(value_type == VALUE_STRING);
            record->SetChar(col->columnName, string_value[0][0]);
        } else if ((col->typeName == CHAR_TYPE && col->length > 1) || col->typeName == VARCHAR_TYPE)
        {
            assert(value_type == VALUE_STRING);
            record->SetString(col->columnName, string_value->c_str());
        } else {
            assert(false);
        }
    }

    Type GetType()
    {
        assert(value_type != VALUE_NULL);
        switch(value_type)
        {
        case VALUE_INT:
            return INT_ENUM;
        case VALUE_STRING:
            return VARCHAR_ENUM;
        default:
            assert(false);
        }
    }
    data_t GetValue()
    {
        switch(GetType())
        {
        case INT_ENUM:
            {
                data_t data = alloc_data(4);
                *(int*)(data->data()) = int_value;
                return data;
            }
        case VARCHAR_ENUM:
            {
                data_t data = alloc_data(string_value->length());
                memcpy(data->data(), string_value->c_str(), string_value->length());
                return data;
            }
        default:
            assert(false);
        }
    }
};

class ValueList
{
public:
    vector<Value*> values;
};
class ValueLists
{
public:
    vector<ValueList*> lists;
};

class InsertStatement : public Statement
{
public:
    string* tbName;
    ValueLists* lists;

    InsertStatement(string* tbName, ValueLists* lists): tbName(tbName), lists(lists) {}

    void run(Context* ctx)
    {
        TableDesc::ptr tb = ctx->dd->SearchTable(*tbName);
        SlotsFile::ptr file = make_shared<SlotsFile>(tb->disk_filename);
        for(auto list: lists->lists)
        {
            Record::ptr record = tb->NewRecord();
            assert(list->values.size() == tb->cols.size());
            for(int i = 0; i < (int)tb->cols.size(); i ++)
            {
                list->values[i]->SetValue(record, tb->Column(i));
            }
            data_t data = record->Generate();
            file->Insert(data);
        }
    }
};

#define OP_EQ 1
#define OP_NEQ 2
#define OP_LE 3
#define OP_GE 4
#define OP_LT 5
#define OP_GT 6
#define OP_IS_NULL 7
#define OP_NOT_NULL 8

class Col
{
public:
    string* tbName;
    string* colName;
    Col(string* colName): tbName(NULL), colName(colName) {}
    Col(string* tbName, string* colName): tbName(tbName), colName(colName) {}

    bool IsNULL(Record::ptr record) const
    {
        assert(tbName == NULL || *tbName == record->td->tableName);
        return record->IsNull(*colName);
    }

    Type GetType(Record::ptr record)
    {
        return record->td->Column(*colName)->typeEnum;
    }
    data_t GetValue(Record::ptr record)
    {
        assert(tbName == NULL || *tbName == record->td->tableName);
        return record->GetValue(*colName);
    }
};

class Expr
{
public:
    Value* value;
    Col* col;
    Expr(Value* value): value(value), col(NULL) {}
    Expr(Col* col): value(NULL), col(col) {}

    Type GetType(Record::ptr record)
    {
        if (value != NULL) return value->GetType();
        return col->GetType(record);
    }
    data_t GetValue(Record::ptr record)
    {
        if (value != NULL) return value->GetValue();
        return col->GetValue(record);
    }
};

class WhereClause
{
public:
    Col* col;
    int op;
    Expr* expr;
    WhereClause(Col* col, int op, Expr* expr = NULL): col(col), op(op), expr(expr) {}

    // return true if ok
    bool Test(Record::ptr record) const
    {
        if (op == OP_IS_NULL || op == OP_NOT_NULL)
        {
            return col->IsNULL(record) == (op == OP_IS_NULL);
        }
        Type type_a = col->GetType(record);
        data_t data_a = col->GetValue(record);
        Type type_b = expr->GetType(record);
        data_t data_b = expr->GetValue(record);
        
        if (data_a == nullptr || data_b == nullptr) return false;
        int cmp = compare(type_a, data_a, type_b, data_b);
        switch(op)
        {
        case OP_EQ: return cmp == 0;
        case OP_NEQ: return cmp != 0;
        case OP_LE: return cmp <= 0;
        case OP_GE: return cmp >= 0;
        case OP_LT: return cmp < 0;
        case OP_GT: return cmp > 0;
        default: assert(false);
        }
    }
};

class WhereClauses
{
public:
    vector<WhereClause*> clauses;

    bool Test(Record::ptr record) const
    {
        bool flag = true;
        for(int i = 0; flag && i < clauses.size(); i ++)
        {
            flag = flag && clauses[i]->Test(record);
        }
        return flag;
    }
};

class DeleteStatement : public Statement
{
public:
    string* tbName;
    WhereClauses* where;
    DeleteStatement(string* tbName, WhereClauses* where): tbName(tbName), where(where) {}

    void run(Context* ctx)
    {
        TableDesc::ptr td = ctx->dd->SearchTable(*tbName);
        SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
        data_t current_data = file->Begin();
        int deleted_count = 0;
        int total_count = 0;
        while(true)
        {
            if (current_data == nullptr) break;
            Record::ptr record = td->RecoverRecord(current_data);
            if (where->Test(record))
            {
                file->Delete(file->CurrentRID());
                deleted_count ++;
            }
            current_data = file->Next();
            total_count ++;
        }
        cout << "========= DELETE ===========" << endl;
        cout << "TOTAL COUNT : " << total_count << endl;
        cout << "DELETED COUNT : " << deleted_count << endl;
    }
};

class SetClause
{
public:
    string* colName;
    Value* value;
    SetClause(string* colName, Value* value): colName(colName), value(value) {}

    void Set(Record::ptr record)
    {
        value->SetValue(record, record->td->Column(*colName));
    }
};
class SetClauses
{
public:
    vector<SetClause*> clauses;

    void Set(Record::ptr record)
    {
        for(int i = 0; i < clauses.size(); i ++)
        {
            clauses[i]->Set(record);
        }
    }
};

class UpdateStatement : public Statement
{
public:
    string* tbName;
    SetClauses* set;
    WhereClauses* where;
    UpdateStatement(string* tbName, SetClauses* set, WhereClauses* where): tbName(tbName), set(set), where(where) {}

    void run(Context* ctx)
    {
        TableDesc::ptr td = ctx->dd->SearchTable(*tbName);
        SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
        data_t current_data = file->Begin();
        int updated_count = 0;
        int total_count = 0;
        vector<data_t> datum_to_append;
        while(true)
        {
            if (current_data == nullptr) break;
            Record::ptr record = td->RecoverRecord(current_data);
            if (where->Test(record))
            {
                file->Delete(file->CurrentRID());
                set->Set(record);
                datum_to_append.push_back(record->Generate());
                updated_count ++;
            }
            current_data = file->Next();
            total_count ++;
        }
        for(data_t x : datum_to_append)
        {
            file->Insert(x);
        }
        cout << "========= UPDATE ===========" << endl;
        cout << "TOTAL COUNT : " << total_count << endl;
        cout << "UPDATED COUNT : " << updated_count << endl;
    }
};

class ColSelector
{
public:
    vector<Col*> cols;
};

class Selector
{
public:
    ColSelector* col_selector;
    Selector(ColSelector* col_selector = NULL): col_selector(col_selector) {}
};

class TableList
{
public:
    vector<string*> tables;
};

class SelectStatement : public Statement
{
public:
    Selector* selector;
    TableList* table_list;
    WhereClauses* where;
    SelectStatement(Selector* selector, TableList* table_list, WhereClauses* where): selector(selector), table_list(table_list), where(where) {}

    void run(Context* ctx)
    {
        assert(selector->col_selector == NULL); // *
        assert(table_list->tables.size() == 1); // 

        TableDesc::ptr td = ctx->dd->SearchTable(*table_list->tables[0]);
        SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
        data_t current_data = file->Begin();
        int selected_count = 0;
        int total_count = 0;
        vector<Record::ptr> records;
        while(true)
        {
            if (current_data == nullptr) break;
            Record::ptr record = td->RecoverRecord(current_data);
            if (where->Test(record))
            {
                records.push_back(record);
                selected_count ++;
            }
            current_data = file->Next();
            total_count ++;
        }
        cout << "========= SELECT ===========" << endl;
        cout << "TOTAL COUNT : " << total_count << endl;
        cout << "SELECTED COUNT : " << selected_count << endl;
        for(Record::ptr x : records)
        {
            x->Output();
        }
    }
};

#endif // FRONTEND_STMTS_TB_H