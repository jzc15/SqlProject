#include <ddf/dbdesc.h>
#include <ddf/record.h>
#include <disk/common.h>
#include <cstdlib>
#include <iostream>
#include <cassert>

using namespace std;

int main()
{
    rmdir("debug");
    mkdirp("debug");

    { // 创建
        DBDesc dd("debug", "debug");
        
        auto student_table = dd.CreateTable("students");
        student_table->CreateColumn("id", "int");
        student_table->CreateColumn("name", "varchar", 10);
        student_table->CreateColumn("age", "int");
        student_table->Finalize();
    
        auto teacher_table = dd.CreateTable("teachers");
        teacher_table->CreateColumn("id", "int");
        teacher_table->CreateColumn("type", "char");
        teacher_table->CreateColumn("name", "varchar", 10);
        teacher_table->CreateColumn("major", "varchar", 50);
        teacher_table->CreateColumn("address", "varchar", 200);
        teacher_table->Finalize();
    
        dd.Save();
    }

    {
        DBDesc dd("debug", "debug");
        auto student_table = dd.SearchTable("students");
        auto teacher_table = dd.SearchTable("teachers");

        auto student_record = student_table->NewRecord();
        student_record->SetInt("id", 10);
        student_record->SetString("name", "jhon");
        student_record->SetInt("age", 20);
        data_t student_data = student_record->Generate();
        cout << student_data->size() << endl;

        auto student_record2 = student_table->RecoverRecord(student_data);
        assert(student_record2->GetInt("id") == 10);
        assert(student_record2->GetString("name") == "jhon");
        assert(student_record2->GetInt("age") == 20);

        auto teacher_record = teacher_table->NewRecord();
        teacher_record->SetInt("id", 20);
        teacher_record->SetChar("type", 'S');
        teacher_record->SetString("name", "Dr.Green");
        teacher_record->SetString("major", "Math");
        teacher_record->SetNull("address");
        data_t teacher_data = teacher_record->Generate();
        cout << teacher_data->size() << endl;

        auto teacher_record2 = teacher_table->RecoverRecord(teacher_data);
        assert(teacher_record2->GetInt("id") == 20);
        assert(teacher_record2->GetChar("type") == 'S');
        assert(teacher_record2->GetString("name") == "Dr.Green");
        assert(teacher_record2->GetString("major") == "Math");
        assert(teacher_record2->IsNull("address"));
    }

    return 0;
}
