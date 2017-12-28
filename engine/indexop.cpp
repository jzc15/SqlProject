#include "indexop.h"
#include "helper.h"
#include <datamanager/slotsfile.h>
#include <indices/bplustree.h>
#include <iostream>

using namespace std;

void create_index(Context* ctx, const string& tb_name, const string& column_name)
{
    TITLE(create_index)
    auto td = ctx->dd->SearchTable(tb_name);
    auto cd = td->Column(column_name);
    int cd_idx = td->ColumnIndex(column_name);
    if (cd->indexed)
    {
        cout << "`" << tb_name << "`.`" << column_name << "` already has index" << endl;
    } else {
        cd->SetIndexed(true);
        BPlusTree::ptr indices = make_shared<BPlusTree>(cd->IndexFilename(), cd->typeEnum);
        SlotsFile::ptr file = make_shared<SlotsFile>(td->disk_filename);
        data_t data = file->Begin();
        while(data != nullptr)
        {
            auto record = td->RecoverRecord(data);
            indices->Insert(record->GetValue(cd_idx), file->CurrentRID());
            data = file->Next();
        }
        cout << "success created index for `" << tb_name << "`.`" << column_name << "`" << endl;
    }
}

void drop_index(Context* ctx, const string& tb_name, const string& column_name)
{
    TITLE(drop_index)
    auto td = ctx->dd->SearchTable(tb_name);
    auto cd = td->Column(column_name);
    if (cd->indexed)
    {
        cd->SetIndexed(false);
        BPlusTree::RemoveIndex(cd->IndexFilename());
        cout << "success drop index for `" << tb_name << "`.`" << column_name << "`" << endl;
    } else {
        cout << "`" << tb_name << "`.`" << column_name << "` does not have index" << endl;
    }
}