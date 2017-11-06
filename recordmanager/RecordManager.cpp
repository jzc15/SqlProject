#include "RecordManager.h"

RecordManager::RecordManager()
{
    this->databaseName = "debug";
}

RecordManager::RecordManager(string databaseName)
{
    this->databaseName = databaseName;
    mkdir(databaseName.c_str() ,0755);
}

bool RecordManager::createRecord(string tableName, bitList buf, ushort length)
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
    ushort dataSize = length;
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
    bitList b = (bitList) bi;
    if(ExistPageCnt < i)                  //创建新页
    {
        if(ExistPageCnt == MAX_SHORT_CNT)
            return false;
        t[0] ++;
        ti[0] = 0;
        ti[1] = PAGE_SIZE;
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
        b[ti[1] + i] = buf[i];
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
bool RecordManager::createRecord(string tableName, RecordBinary rb)
{
    return createRecord(tableName, rb.ptr, (rb.size));
}
RecordBinary RecordManager::searchRecord(string tableName, ushort PageNo, ushort SlotNo)
{
    FileManager f;
    RecordBinary u;
    u.size = 0;
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
    
    
    if(t[0] < PageNo || PageNo <=  0)
        return u;
    //cout << "PageNo = " << PageNo << endl;
    //cout << "SlotNo = " << SlotNo << endl;
    /*
     * 获取第i页，判断SlotNo是否合法，查询数据
     */
    BufType bi = bpm.getPage(fileID, PageNo, index);
    bpm.access(index);
    ushortList ti = (ushortList) bi;
    bitList b = (bitList) bi;
    if(ti[0] < SlotNo || PageNo <= 0)
        return u;

    ushort length = ti[SlotNo*2];
    ushort startPos = ti[SlotNo*2+1];
    
    u.ptr = new uchar[length];
    u.size = length;
    for(ushort i = 0; i < length; i++)
        u.ptr[i] = b[startPos+i];
    
    // cout << "length = " << length << endl;
    // cout << "startPos = " << startPos << endl;
    // for(ushort i = 12; i < 50; i++)
    //     cout << (int)u.ptr[i];
    // cout << endl;
    bpm.close();
    f.closeFile(fileID);
    return u;
}

bool RecordManager::deleteRecord(string tableName, ushort PageNo, ushort SlotNo)
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
    
    if(t[0] < PageNo || PageNo <= 0)
        return false;
    
    /*
     * 获取第i页，判断SlotNo是否合法，查询数据
     */
    BufType bi = bpm.getPage(fileID, PageNo, index);
    bpm.markDirty(index);
    ushortList ti = (ushortList) bi;
    bitList b = (bitList) bi;

    if(ti[0] < SlotNo || PageNo <= 0)
        return false;
    
    ushort length = ti[SlotNo*2];
    ushort startPos = ti[SlotNo*2+1];
    t[PageNo] -= (length * 2 + 4);    // 维护首页
    ti[0] --;
    ti[1] += length;
    for(ushort i = ti[SlotNo*2+1] + ti[SlotNo*2] - 1; i >= ti[1]; i--)
        b[i] = b[i-length];
    for(ushort i = SlotNo; i <= ti[0]; i++)
    {
        ti[i*2] = ti[i*2+2];
        ti[i*2+1] = ti[i*2+3] + length;
    }
    //cout << "ti[5] = " << ti[5] << endl;
    //cout << "SlotNo = " << SlotNo << endl;
    bpm.close();
    f.closeFile(fileID);
    return true;
}

bool RecordManager::nextRecord(string tableName, ushort& PageNo, ushort& SlotNo)
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
        return false;
    
    /*
     * 获取第i页，判断SlotNo是否合法，查询数据
     */
    BufType bi = bpm.getPage(fileID, PageNo, index);
    bpm.access(index);
    ushortList ti = (ushortList) bi;
    
    if(ti[0] < SlotNo || PageNo <= 0)
        return false;
    
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

ushort RecordManager::getPageCnt(string tableName)
{
    FileManager f;
    RecordBinary u;
    u.size = 0;
    string filepath = databaseName + "/" + tableName;
    int fileID;
    f.createFile(filepath.c_str());
    f.openFile(filepath.c_str(), fileID);
    BufPageManager bpm(&f);
    int index;
    /*
     * 获取第0页，返回总页数
     */
    BufType b0 = bpm.getPage(fileID, 0, index);
    bpm.access(index);
    return b0[0];
}

bool RecordManager::saveRecord(string tableName, RecordBinary rb,  ushort PageNo, ushort SlotNo)
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
    //cout << "t[0] = " << t[0] << endl;
    //cout << "PageNo = " << PageNo << endl;
    if(t[0] < PageNo || PageNo <= 0)
        return createRecord(tableName, rb);

    BufType bi = bpm.getPage(fileID, PageNo, index);
    bpm.markDirty(index);
    ushortList ti = (ushortList) bi;
    bitList b = (bitList) bi;
    if(ti[0] < SlotNo || PageNo <= 0)
        return createRecord(tableName, rb);

    ushort length = ti[SlotNo*2];
    ushort startPos = ti[SlotNo*2+1];
    
    if(length != rb.size)
        return false;

    for(ushort i = 0; i < length; i++)
        b[startPos+i] = rb.ptr[i];
    
    // cout << "length = " << length << endl;
    // cout << "startPos = " << startPos << endl;
    // for(ushort i = 12; i < 50; i++)
    //     cout << (int)b[startPos+i];
    // cout << endl;
    bpm.close();
    f.closeFile(fileID);
    return true;

}