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
        cout << x << endl;
    }
}

void create_table(Context* ctx, 
    const string& tb_name,
    const vector<ColumnDefine>& cols,
    const vector<string>& primary_cols,
    const vector<ColumnForeign>& foreign_cols)
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
            cerr << "Primary key should be fix size data type." << endl;
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
            cerr << "ERROR ON CREATE TABLE : foreign key is not a primary key" << endl;
            return;
        }
        if (ref_td->PrimaryIdxs()->size() != 1u)
        {
            cerr << "ERROR ON CREATE TABLE : foreign key is not the only primary key" << endl;
            return;
        }
        ColDesc::ptr c = td->Column(foreign.col_name);
        if (c->typeEnum != f->typeEnum)
        {
            cerr << "ERROR ON CREATE TABLE : foreign key type is not the same" << endl;
            return;
        }
        c->SetForeignKey(foreign.ref_tb_name, foreign.ref_col_name);
    }
    td->Finalize();
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
}

void desc_table(Context* ctx, const string& tb_name)
{
    TITLE(desc_table);
    auto td = ctx->dd->SearchTable(tb_name);

    for(auto c : td->cols)
    {
        cout << c->columnName << " " << c->typeName << endl;
    }
}
