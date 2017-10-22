#include <ddf/DatabaseDescription.h>
#include <common/Record.h>
#include <cstdlib>

using namespace std;

int main()
{
    system("rm -rf debug");

    { // 创建
        DatabaseDescription dd("debug");
        
        auto student_table = dd.CreateTable("students");
        student_table->CreateColumn("id", "int");
        student_table->CreateColumn("name", "varchar", 10);
        student_table->CreateColumn("age", "int");
        student_table->Finalize();
    
        auto teacher_table = dd.CreateTable("teachers");
        teacher_table->CreateColumn("id", "int");
        teacher_table->CreateColumn("name", "varchar", 10);
        teacher_table->CreateColumn("major", "varchar", 50);
        teacher_table->Finalize();
    
        dd.Save();
    }

    {
        DatabaseDescription dd("debug");
        auto student_table = dd.SearchTable("students");
        auto teacher_table = dd.SearchTable("teachers");

        auto student_record = student_table->NewRecord();
        student_record->SetInt("id", 10);
        student_record->SetString("name", "jhon");
        student_record->SetInt("age", 20);
    }

    return 0;
}
