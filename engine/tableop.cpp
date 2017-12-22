#include "tableop.h"
#include "helper.h"
#include <indices/bplustree.h>
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
    if (primary_cols.size() > 1u)
    {
        cerr << "ERROR ON CREATE TABLE : multi primary key" << endl;
        return;
    }

    TableDesc::ptr td = ctx->dd->CreateTable(tb_name);
    for(auto col : cols)
    {
        td->CreateColumn(col.col_name, type_name(col.type.type), col.type.length, col.allow_null);
    }
    td->Finalize();

    if (primary_cols.size() != 0u)
    {
        td->Column(primary_cols[0])->SetPrimary();
    }
    td->Finalize();
    
    for(auto foreign : foreign_cols)
    {
        ColDesc::ptr f = ctx->dd->SearchTable(foreign.ref_tb_name)->Column(foreign.ref_col_name);
        if (!f->is_primary)
        {
            cerr << "ERROR ON CREATE TABLE : foreign key is not a primary key" << endl;
            return;
        }
        ColDesc::ptr c = td->CreateColumn(foreign.col_name, f->typeName, f->Length(), false);
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
        if (c->is_primary) BPlusTree::RemoveIndex(c->PrimaryFilename());
    }
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
