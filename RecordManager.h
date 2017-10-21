#ifndef RECORD_MANAGER
#define RECORD_MANAGER

#include "filesystem/bufmanager/BufPageManager.h"
#include "filesystem/fileio/FileManager.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#define SHORT_MAX 65536
#define HIGH true
#define LOW false
#define MAX_SHORT_CNT 4095

using namespace std;
/*
 *  接口总结
 *  RecordManger(string databaseName)   
 *      构造一个数据库，创建数据库文件夹
 *  createRecord(string tableName, ushortList buf, BufLength length)  
 *      添加数据，如果数据表不存在则创建数据表，返回值表示是否添加成功
 *  searchRecord(string tableName, ushort PageNo, ushort SlotNo, ushort &length)
 *      根据页编号和槽编号查询数据，返回ushortList指针，以及数据长度
 *  deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
 *      根据页编号和槽编号查询数据，删除数据
 *  nextRecord(string tableName, ushort& PageNo, ushort& SlotNo)
 *      根据页编号和槽编号查询下一条数据，返回是否存在下一条
 */
class RecordManager
{
    string databaseName;
public:
    RecordManager()
    {
        this->databaseName = "debug";
    }

    RecordManager(string databaseName)
    {
        this->databaseName = databaseName;
        string dirname = databaseName;
        mkdir(dirname.c_str() ,0755);
        
    }

    bool createRecord(string tableName, ushortList buf, BufLength length)
    {
        FileManager f;
        string filepath = databaseName + "/" + tableName;
        int fileID;
        f.createFile(filepath.c_str());
        f.openFile(filepath.c_str(), fileID);
        BufPageManager bpm(&f);
        int index;
        /*
         * 获取第0页，其中第一个short记录这一页已经用了多少，每页堆信息从第二位开始顺序存储, 维护首页
         * 数据页编号从1开始
         */
        BufType b0 = bpm.getPage(fileID, 0, index);
        bpm.markDirty(index);
        ushortList t = (ushortList) b0;
        
        ushort ExistPageCnt;
        ExistPageCnt = t[0];
        ushort i = 1;
        ushort dataSize = length * BufLengthSize;
        ushort bothSize = dataSize + 4;
        while(i <= ExistPageCnt)
        {
            if(t[i] + bothSize <= PAGE_SIZE)
                break;
            i++;
        }
        /*
        cout << "t[0]" << " = " << t[0] << endl;
        cout << "i" << " = " << i << endl;
        cout << "t[i]" << " = " << t[i] << endl;
        cout << "bothsize" << " = " << bothSize << endl;
        */
        BufType bi = bpm.getPage(fileID, i, index);
        bpm.markDirty(index);
        ushortList ti = (ushortList) bi;

        if(ExistPageCnt < i)                  //创建新页
        {
            if(ExistPageCnt == MAX_SHORT_CNT)
                return false;
            t[0] ++;
            ti[0] = 0;
            ti[1] = 4096;
            t[i] = 4;
        }
        t[i] += bothSize;
        /*
         * 修改第i页，倒序存储数据，正序存储表头
         * 槽编号从1开始
         */
        
        ti[0] ++;
        ti[1] -= length;
        for(i = 0; i < length; i++)
            ti[ti[1] + i] = buf[i];
        /*
        cout << "ti[0]" << " = " <<  ti[0] << endl;
        cout << "ti[1]" << " = " << ti[1] << endl;
        */
        ti[ti[0]*2] = length;
        ti[ti[0]*2+1] = ti[1];
        
        bpm.close();
        f.closeFile(fileID);
        return true;
    }

    ushortList searchRecord(string tableName, ushort PageNo, ushort SlotNo, ushort &length)
    {
        FileManager f;
        string filepath = databaseName + "/" + tableName;
        int fileID;
        f.createFile(filepath.c_str());
        f.openFile(filepath.c_str(), fileID);
        BufPageManager bpm(&f);
        int index;
        /*
         * 获取第0页，检查PageNo是否合法
         */
        BufType b0 = bpm.getPage(fileID, 0, index);
        bpm.access(index);
        ushortList t = (ushortList) b0;
        
        if(b0[0] < PageNo || PageNo <= 0)
            return NULL;
        
        /*
         * 获取第i页，判断SlotNo是否合法，查询数据
         */
        BufType bi = bpm.getPage(fileID, PageNo, index);
        bpm.access(index);
        ushortList ti = (ushortList) bi;
        
        if(ti[0] < SlotNo || PageNo <= 0)
            return NULL;
        
        length = ti[SlotNo*2];
        ushort startPos = ti[SlotNo*2+1];
        ushortList u = new ushort[length];

        for(ushort i = 0; i < length; i++)
            u[i] = ti[startPos+i];
        bpm.close();
        f.closeFile(fileID);
        return &u[0];
    }

    bool deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
    {
        FileManager f;
        string filepath = databaseName + "/" + tableName;
        int fileID;
        f.createFile(filepath.c_str());
        f.openFile(filepath.c_str(), fileID);
        BufPageManager bpm(&f);
        int index;
        /*
         * 获取第0页，检查PageNo是否合法
         */
        BufType b0 = bpm.getPage(fileID, 0, index);
        bpm.markDirty(index);
        ushortList t = (ushortList) b0;
        
        if(b0[0] < PageNo || PageNo <= 0)
            return NULL;
        
        /*
         * 获取第i页，判断SlotNo是否合法，查询数据
         */
        BufType bi = bpm.getPage(fileID, PageNo, index);
        bpm.markDirty(index);
        ushortList ti = (ushortList) bi;
        
        if(ti[0] < SlotNo || PageNo <= 0)
            return NULL;
        
        ushort length = ti[SlotNo*2];
        ushort startPos = ti[SlotNo*2+1];

        t[PageNo] -= (length * 2 + 4);    // 维护首页
        ti[0] --;
        ti[1] += length;
        for(ushort i = ti[SlotNo*2+1] + ti[SlotNo*2] - 1; i >= ti[1]; i--)
            ti[i] = ti[i-length];
        for(ushort i = SlotNo; i <= ti[0]; i++)
        {
            ti[i*2] = ti[i*2+2];
            ti[i*2+1] = ti[i*2+3] + length;
        }
        cout << "ti[5] = " << ti[5] << endl;
        cout << "SlotNo = " << SlotNo << endl;
        bpm.close();
        f.closeFile(fileID);
        return true;
    }

    bool nextRecord(string tableName, ushort& PageNo, ushort& SlotNo)
    {
        FileManager f;
        string filepath = databaseName + "/" + tableName;
        int fileID;
        f.createFile(filepath.c_str());
        f.openFile(filepath.c_str(), fileID);
        BufPageManager bpm(&f);
        int index;
        /*
         * 获取第0页，检查PageNo是否合法
         */
        BufType b0 = bpm.getPage(fileID, 0, index);
        bpm.access(index);
        ushortList t = (ushortList) b0;
        
        if(b0[0] < PageNo || PageNo <= 0)
            return NULL;
        
        /*
         * 获取第i页，判断SlotNo是否合法，查询数据
         */
        BufType bi = bpm.getPage(fileID, PageNo, index);
        bpm.access(index);
        ushortList ti = (ushortList) bi;
        
        if(ti[0] < SlotNo || PageNo <= 0)
            return NULL;
        
        while(true)
        {
            if(SlotNo < ti[0])
            {
                SlotNo ++;
                return true;
            }
            else if (PageNo < t[0])
            {
                PageNo ++;
                BufType bi = bpm.getPage(fileID, PageNo, index);
                bpm.access(index);
                ushortList ti = (ushortList) bi;
                SlotNo = 0;
            }
            else
                return false;
        }
        bpm.close();
        f.closeFile(fileID);
        return false;
    }
};

#endif