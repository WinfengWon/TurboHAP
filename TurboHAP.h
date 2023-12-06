#include <iostream>
#include<assert.h>
#include <vector>
using namespace std;
#define NodeNumber 70
#define FUtypeNumber 2 //FU类型的最大数量
#define EntryNumber 2 //每个FU类型允许的最大entry数量
#define TimeColum 10 //允许每个node的最高时间
#define DebugTableB false //设置是否输出表格B
#define GAMA 1e-30 //定义比较概率时候的精确度
//存放输入数据(time,probability,cost)
struct TimProCos {
    int time;
    double probability;
    int cost;
    TimProCos(int t=0, double p=0, int c=0) {
        time = t;
        probability = p;
        cost = c;
    }
};
TimProCos Data[NodeNumber][FUtypeNumber][EntryNumber];//用于存放数据

struct Event{
    vector<unsigned int> bitmaps;
    double prob;
    unsigned int start; //记录当前的事件序列的起始节点，如步长为3时：0、1、2为一组；3、4、5为一组。
    unsigned int end;   //记录当前的事件序列的结束节点
    vector<int> FUtypeArr;
    Event(){
        bitmaps.push_back(0);
        prob=1.0;
    }

    /*输入type和n，前者代表FU类型，后者代表事件
    其中，0代表初始化事件0，1代表初始化事件0、1。*/
    void initial(unsigned int FUtype,unsigned int n,unsigned int s)
    {
        SetBit(0,n);
        start=s;
        end=s;
        FUtypeArr.push_back(FUtype);
    }

    //设置给定第n个bit为1，n从0开始计
    bool SetBit(unsigned int n){
        while(n/32+1>bitmaps.size())
        {
            bitmaps.push_back(0);
        }
        bitmaps[n/32]=bitmaps[n/32]|1<<n%32;
        return true;
    }
    //设置给定第n个bit为0，n从0开始计
    bool ResetBit(unsigned int n){
        assert(n/32<bitmaps.size());//由于n/32是从0开始计算的，那么它应该小于bitmaps的元素数
        bitmaps[n/32]=bitmaps[n/32]&~(1<<n%32);
        return true;
    }
    //从l位到r位全部置位(包括l和r本身)
    bool SetBit(unsigned int l,unsigned int r){
        while(r/32+1>bitmaps.size())
            bitmaps.push_back(0);
        for(unsigned int i=l;i<=r;i++)
            SetBit(i);
        return true;
    }
    //按照16进制的格式，打印Event。越下越右边
    void Print(){
        for(vector<unsigned int>::iterator i=bitmaps.begin();i<bitmaps.end();i++)
            printf("%#.8X\n",*i);
    }
    void Update(int FUtype,unsigned int flag){//前面FU类型，后面事件。传入0代表当前位置选择事件0，传入1代表当前位置选择事件1、0
        unsigned int length=bitmaps.size()*32;
        Event new_bitmaps;
        for(unsigned int i=0;i<length;i++){
            if(bitmaps[i/32]&1<<i%32){
                if(flag==1){
                    unsigned int n=2*i;
                    new_bitmaps.SetBit(n);
                    new_bitmaps.SetBit(n+1);
                }
                else{
                    unsigned int n=2*i; 
                    new_bitmaps.SetBit(n);
                }
            }
        }
        FUtypeArr.push_back(FUtype);
        end++;
        bitmaps=move(new_bitmaps.bitmaps);
        return;
    }

};
struct Btable:Event
{
    int cost;
    int time;
    int FUtype;//FU类型
    int entry; //事件
    Btable(){}
    void UnionTwoE(vector<unsigned int> &rbitmaps)//算得的概率放在this->prob
    {
        unsigned int length=rbitmaps.size()*32;
        for(int i=0;i<length;i++)
        {
            if(rbitmaps[i/32]&1<<i%32&&!(bitmaps[i/32]&1<<i%32))
            {
                prob+=Add_prob(i);
                SetBit(i);
            }
        }
    }
    double Add_prob(int n)
    {
        double res=1.0;
        for(int i=end-start;i>-1;i--)
        {
            int entry_idx=(n&1<<i)>0?1:0;
            res*=Data[end-i][FUtypeArr[end-start-i]][entry_idx].probability;
        }
        return res;
    }
    Btable circle_multiply(Btable &r)
    {
        Btable b=*this;
        b.cost+=r.cost;
        b.time+=r.time;
        b.prob*=r.prob;
        b.Update(r.FUtype,r.entry);
        return b;
    }
};


//定义全局变量
// list <Btable> D[NodeNumber][TimeColum*NodeNumber]; //表格允许的最高时间=TimeColum*NodeNumber
// list <Btable> B[NodeNumber][TimeColum];

list<vector<list<Btable>>> B;//B就代替了之前的B和D，B最外层是list，当合并的时候，就删除掉被合并的节点。

int EntryNumOfData[NodeNumber][FUtypeNumber]={0};//记录每个node的每个FU类型有多少种Time可能
int MaxTimeOfB[NodeNumber]={0};
int Node_num;//实际输入的node数量
int count_equal = 0;