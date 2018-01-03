#include "tableop.h"
#include "helper.h"
#include <indices/bplustree.h>
#include <indices/hashtable.h>
#include <iostream>

using namespace std;

void show_tables(Context* ctx)
{
    TITLE(show_tables);
    for(auto x : ctx->dd->TableList())
    {
        *out << x << endl;
    }
}

void create_table(Context* ctx, 
    const string& tb_name,
    const vector<ColumnDefine>& cols,
    const vector<string>& primary_cols,
    const vector<ColumnForeign>& foreign_cols,
    const vector<ScopeLimit>& scope_limits)
{
    TableDesc::ptr td = ctx->dd->CreateTable(tb_name);
    for(auto col : cols)
    {
        td->CreateColumn(col.col_name, type_name(col.type.type), col.type.length, col.allow_null);
    }
    td->Finalize();

    for(auto col : primary_cols)
    {
        if (!td->Column(col)->fixed) {
            *err << "Primary key should be fix size data type." << endl;
            return;
        }
        td->Column(col)->SetOneOfPrimary();
    }
    td->Finalize();
    
    for(auto foreign : foreign_cols)
    {
        TableDesc::ptr ref_td = ctx->dd->SearchTable(foreign.ref_tb_name);
        ColDesc::ptr f = ref_td->Column(foreign.ref_col_name);
        if (!f->is_oneof_primary)
        {
            *err << "ERROR ON CREATE TABLE : foreign key is not a primary key" << endl;
            return;
        }
        if (ref_td->PrimaryIdxs()->size() != 1u)
        {
            *err << "ERROR ON CREATE TABLE : foreign key is not the only primary key" << endl;
            return;
        }
        ColDesc::ptr c = td->Column(foreign.col_name);
        if (c->typeEnum != f->typeEnum)
        {
            *err << "ERROR ON CREATE TABLE : foreign key type is not the same" << endl;
            return;
        }
        c->SetForeignKey(foreign.ref_tb_name, foreign.ref_col_name);
    }
    td->Finalize();

    // 域约束
    for(auto scope_limit : scope_limits)
    {
        vector<Json> values;
        auto cd = td->Column(scope_limit.col_name);
        for(auto v : scope_limit.values)
        {
            if (v.value_type == Value::VALUE_NULL)
            {
                *err << "ERROR ON CREATE TABLE : scope limit value can not be NULL" << endl;
                return;
            }
            values.push_back(v.basic_to_json());
            if (!value_type_trans_ok(cd->typeEnum, v))
            {
                *err << "ERROR ON CREATE TABLE : wrong type of scope limit value : " << v.stringify() << " for column `" << scope_limit.col_name << "`" << endl;
                return;
            }
        }
        cd->SetScopeValues(values);
    }
    td->Finalize();

    ctx->dd->Finalize();
}

void drop_table(Context* ctx, const string& tb_name)
{
    auto td = ctx->dd->DropTable(tb_name);
    ctx->dd->Save();

    if (exists(td->disk_filename)) rmfile(td->disk_filename);
    for(auto c : td->cols)
    {
        if (c->indexed) BPlusTree::RemoveIndex(c->IndexFilename());
    }
    HashTable::RemoveIndex(td->PrimaryFilename());

    ctx->dd->Finalize();
}

void desc_table(Context* ctx, const string& tb_name)
{
    TITLE(desc_table);
    auto td = ctx->dd->SearchTable(tb_name);

    for(auto c : td->cols)
    {
        *out << c->columnName << " " << c->typeName << endl;
    }
}
