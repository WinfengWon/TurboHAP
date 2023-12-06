#include <iostream>
#include <vector>
#include<list>
#include <algorithm>
#include <assert.h>
#include<math.h>
#include <chrono>
//宏定义
#define NodeNumber 70
#define FUtypeNumber 2 //FU类型的最大数量
#define EntryNumber 2 //每个FU类型允许的最大entry数量
#define TimeColum 20 //允许每个node的最高时间
#define DebugTableB true //设置是否输出表格B
#define GAMA 1e-30
using namespace std;

struct ProbAndCostPair
{
    double prob;
    int cost;
    ProbAndCostPair(double p, int c) {
        prob = p;
        cost = c;
    }
};
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
//定义全局变量
list <ProbAndCostPair> D[NodeNumber][TimeColum*NodeNumber]; //表格允许的最高时间=TimeColum*NodeNumber
list <ProbAndCostPair> B[NodeNumber][TimeColum];//表格B，存放每个node自身信息
TimProCos Data[NodeNumber][FUtypeNumber][EntryNumber];
int EntryNumOfData[NodeNumber][FUtypeNumber]={0};//记录每个node的每个FU类型有多少种Time可能
int MaxTimeOfB[NodeNumber]={0};
int Node_num;//实际输入的node数量
int count_equal = 0;

void inputData() {
    cin >> Node_num;
    for (int i = 0; i < Node_num; i++) {
        int FU_num; cin >> FU_num;//当前node有多少个FU类型
        for (int j = 0; j < FU_num; j++) {
            int entry_num; cin >> entry_num;//当前FU类型有多少个输入(即有多少种Time可能)
            EntryNumOfData[i][j]=entry_num;
            for (int k = 0; k < entry_num; k++) {
                int time, cost;
                double probability;
                cin >> time >> probability >> cost;
                Data[i][j][k].time = time;
                Data[i][j][k].probability = probability;
                Data[i][j][k].cost = cost;
            }
        }
    }
}
bool compare(const ProbAndCostPair &a, const ProbAndCostPair &b)
{
    return a.prob < b.prob;
}
/*
* 参数用的是引用
*/
void removal(list<ProbAndCostPair>& L)
{
    L.sort(compare);
    for (list<ProbAndCostPair>::iterator i = L.begin(); i != --L.end();) 
    {
        list<ProbAndCostPair>::iterator left=i,right = i;
        right++;
        if (fabs(left->prob-right->prob)<GAMA)
        {
            if (left->cost >= right->cost)
            {
                if (i == L.begin()) i++;
                else i--;
                L.erase(left);
            }
            else
            {
                L.erase(right);
            }
        }
        else
        {
            if (left->cost >= right->cost)
            {
                if (left->cost == right->cost) count_equal++;
                if (i == L.begin()) i++;
                else i--;
                L.erase(left);
            }
            else
            {
                i++;
            }
        }
    }
    return;
}
void PrintPair(list<ProbAndCostPair>& L)
{
    for (list<ProbAndCostPair>::iterator i = L.begin(); i != L.end(); i++)
        printf("<%f,%d>...", i->prob, i->cost);
}

void creatTableB() {
    for (int i = 0; i < Node_num; i++) {
        //Bentry放累加之后的项目，BentryBuffer用来进行累加；
        list <TimProCos> Bentry;
        TimProCos a;
        double accumulate;
        int time=0;//记录当前node的最高时间
        for (int j = 0; j<FUtypeNumber; j++){
            accumulate=0; //每次一个新类型，都需要归零accumulate   
            for (int k = 0; k < EntryNumOfData[i][j]; k++) {
                a = Data[i][j][k];
                accumulate+=Data[i][j][k].probability;
                a.probability=accumulate;
                // assert(abs(a.probability - 1) < 1e-3);
                Bentry.insert(Bentry.end(),a);
            }
        }
        //循环之后，Bentry里面放的是一个node的所有类型的所有entry
        for(list<TimProCos>::iterator it=Bentry.begin();it!=Bentry.end();it++)
        {
            //time以1为开始，也即0列什么也不存
            if(it->time>time)   time=it->time;
            B[i][it->time].push_back(ProbAndCostPair(it->probability,it->cost));
        }
        MaxTimeOfB[i]=time;    
        for(int t=2;t<=time;t++)
        {
            B[i][t].insert(B[i][t].end(),B[i][t-1].begin(),B[i][t-1].end());//把前面一个格子的数据加到当前格子
            removal(B[i][t]);
            if(i==0)
            D[i][t]=B[i][t];//让D1,j=B1,j
        }
        if(i==0) D[0][1]=B[0][1];//上面t从2开始，需要补上这个1
        //以下模块用于输出Table B的信息
/*         if(DebugTableB){
            printf("node %d:\n",i);
            for(int t=1;t<=time;t++)
            {
                printf("time=%d: ",t);
                for(list<ProbAndCostPair>::iterator it=B[i][t].begin();it!=B[i][t].end();it++)
                printf("(%f,%d)",it->prob,it->cost);
                printf("\n");
            }
        } */
    }  
}
list<ProbAndCostPair> Circle_Multiply(list<ProbAndCostPair>& L1,list<ProbAndCostPair> L2){
    list<ProbAndCostPair> L;
    for(list<ProbAndCostPair>::iterator i=L1.begin();i!=L1.end();i++)
        for(list<ProbAndCostPair>::iterator j=L2.begin();j!=L2.end();j++){
            L.push_back(ProbAndCostPair(i->prob*j->prob,i->cost+j->cost));
        }
    return L;
}
void Path_Assign() {  
    int t=1,min_t;
    for(int i=1;i<Node_num;i++){       
        min_t=TimeColum*NodeNumber+1;
        for(;t<TimeColum*NodeNumber;t++){
            for(int k=1;k<=MaxTimeOfB[i];k++){
                if(B[i][k].empty()) continue;//如果B[i][k]对应的格子为空，则跳过本轮
                if(t-k<1) break;//如果t-k<=0，则结束本循环
                if(D[i-1][t-k].empty()) continue;
                else{
                    if(min_t>t) min_t=t;//记录本轮最小的t
                    list<ProbAndCostPair> L=Circle_Multiply(D[i-1][t-k],B[i][k]);
                    D[i][t].insert(D[i][t].end(),L.begin(),L.end());
                }
            }
            if(D[i][t].empty()) continue;
            D[i][t].insert(D[i][t].end(),D[i][t-1].begin(),D[i][t-1].end());
            removal(D[i][t]);
        }
        t=min_t;
    }

}
int main(int argc, char* argv[])
{
    inputData();
    auto start_time = std::chrono::high_resolution_clock::now();
    creatTableB();
    Path_Assign();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "program executing time" << duration.count() << "ms" << std::endl;


    // int i=Node_num-1;
    // for(int t=1;t<=TimeColum*NodeNumber;t++)
    // {
    //     if(D[i][t].empty()) continue;
    //     printf("time=%d: ",t);
    //     for(list<ProbAndCostPair>::iterator it=D[i][t].begin();it!=D[i][t].end();it++)
    //     printf("(%f,%d)",it->prob,it->cost);
    //     printf("\n");
    // }


    // string X="[",Y="[",Z="[";//用X,Y,Z存放坐标值
    // for(int x=1;x<=TimeColum*NodeNumber;x++)
    // {
    //     if(D[Node_num-1][x].empty()) continue;
    //     for(auto &i:D[Node_num-1][x])
    //     {
    //         X+=to_string(x)+",";
    //         Y+=to_string(i.cost)+",";
    //         Z+=to_string(i.prob)+",";
    //     }
    // }
    // X.pop_back();
    // Y.pop_back();
    // Z.pop_back();
    // X+="]";
    // Y+="]";
    // Z+="]";
    // cout<<X<<'\n';
    // cout<<Y<<'\n';
    // cout<<Z<<'\n';
    return 0;
}

/*
input:
total node number N
FU_type number for node 0
entry number for FU_type 0
T P C
T P C
entry number for FU_type 1
T P C
T P C
FU_type number for node 1
entry number for FU_type 0
...


3
2
2
1 0.9 10
2 0.1 10
2
2 0.7 4
4 0.3 4
2
2
1 0.8 8
2 0.2 8
2
3 0.6 3
4 0.4 3
2
2
2 0.65 11
3 0.35 11
2
4 0.9 5
5 0.1 5


*/

