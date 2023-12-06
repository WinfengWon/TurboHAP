/* 
1.block
2. sort by costs
3. without appending
4. parallel
 */
#include <iostream>
#include <vector>
#include<assert.h>
#include <algorithm>
#include <list>
#include <pthread.h>
#include <thread>
#include <chrono>
#include<math.h>
#include "TurboHAP.h"
using namespace std;
void inputData() {

    cin >> Node_num;
    B.resize(Node_num);
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
                MaxTimeOfB[i]=MaxTimeOfB[i]<time?time:MaxTimeOfB[i];
            }
        }
    }
}

/*
* 参数用的是引用
*/
void Newremoval(list<Btable>& L)
{
    for (list<Btable>::iterator i = L.begin(); i != --L.end();) 
    {
        list<Btable>::iterator left=i,right = i;
        right++;
        if (!(left->prob<right->prob))
        {
            L.erase(right);
        }
        else
        {
            if (left->cost == right->cost)
            {
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
// bool compareProb(const Btable &a, const Btable &b)
// {
//     return a.prob < b.prob;
// }
bool compareCost(const Btable &a, const Btable &b)
{
    return a.cost < b.cost;
}
void removal(list<Btable>& L)
{
    L.sort(compareCost);
    Newremoval(L);
}
void BitMapRemoval(list<Btable>& L)
{
    L.sort(compareCost);
    for(list<Btable>::iterator i = L.begin(); i !=L.end();i++)
    for(list<Btable>::iterator j = i; j !=L.end();j++)
    {
        if(i->cost==-1) break;
        if(i->cost!=j->cost) break;
        if(i==j) continue;
        if(j->cost==-1) continue;
        assert(i->FUtypeArr.size()==j->FUtypeArr.size());

        int x=0;
        for(;x<i->FUtypeArr.size();x++) //判断是否整个assignments都相等
        {
            if(i->FUtypeArr[x]!=j->FUtypeArr[x]) break;
        }
        if(x!=i->FUtypeArr.size()) continue;

        if(i->bitmaps.size()>j->bitmaps.size())//没有注意到i和j的bitmaps长度关系，可能会造成段错误(越界访问)
        {
            i->UnionTwoE(j->bitmaps);
            if(i->prob>1) i->prob=1.0;
            j->cost=-1;
        }
        else
        {
            j->UnionTwoE(i->bitmaps);
            if(j->prob>1) j->prob=1.0;
            i->cost=-1;
        }

    }
    for(list<Btable>::iterator i = L.begin(); i !=L.end();)
    {
        auto j=i;
        j++;
        if(i->cost==-1) {L.erase(i);}
        i=j;
    }
    return;
}
void PrintPair(list<Btable>& L)
{
    for (list<Btable>::iterator i = L.begin(); i != L.end(); i++)
        printf("<%f,%d>...", i->prob, i->cost);
}

void creatTableB() {
    list<vector<list<Btable>>>::iterator B_iter=B.begin();
    for (int i = 0; i < Node_num; i++,B_iter++) 
    {       
        B_iter->resize(MaxTimeOfB[i]+1);//由于vector从0开始，所以要比最大时间+1进行初始化
        for (int j = 0; j<FUtypeNumber; j++)
        {
            double accumulate=0; //每次一个新类型，都需要归零accumulate   
            
            for (int k = 0; k < EntryNumOfData[i][j]; k++) {
                Btable a;
                a.cost = Data[i][j][k].cost;
                a.time = Data[i][j][k].time;
                a.FUtype=j;
                a.entry=k;
                accumulate+=Data[i][j][k].probability;
                a.prob=accumulate;
                // assert(abs(a.probability - 1) < 1e-3);
                a.initial(j,k,i); //初始化从Event继承来的成员，j表示此时FUtypeArr插入的第一个元素，k表示当前选择的事件，i表示start节点，end也初始化为start
                
                (*B_iter)[a.time].push_back(a);
            }
        }

        for(int t=2;t<=MaxTimeOfB[i];t++)
        {
            (*B_iter)[t].insert((*B_iter)[t].end(),(*B_iter)[t-1].begin(),(*B_iter)[t-1].end());//把前面一个格子的数据加到当前格子
            removal((*B_iter)[t]);
        }

        // 以下模块用于输出Table B的信息
        if(DebugTableB){
            printf("node %d:\n",i);
            for(int t=1;t<=MaxTimeOfB[i];t++)
            {
                printf("time=%d: ",t);
                for(auto &it:(*B_iter)[t])
                printf("(%f,%d,%d,%d)",it.prob,it.cost,it.FUtype,it.entry);
                printf("\n");
            }
        }
        
    }  
    return;
}
list<Btable> Circle_Multiply(list<Btable>& L1,list<Btable>& L2){ //把两个Btable列表相乘
    list<Btable> L;
    for(list<Btable>::iterator i=L1.begin();i!=L1.end();i++)
        for(list<Btable>::iterator j=L2.begin();j!=L2.end();j++){
            L.push_back(i->circle_multiply(*j));//Btable.circle_multiply返回的是一个Btable对象，不改变这里的i和j
        }
    return L;
}

list<Btable> Circle_Multiply_Without_Bitmap(list<Btable>& L1,list<Btable>& L2){
    list<Btable> L;
    for(list<Btable>::iterator i=L1.begin();i!=L1.end();i++)
        for(list<Btable>::iterator j=L2.begin();j!=L2.end();j++){
            Btable b;
            b.cost=i->cost+j->cost;
            b.time=i->time+j->time;
            b.prob=i->prob*j->prob;
            L.push_back(b);
        }
    return L;
}

void thread_function(list<vector<list<Btable>>>::iterator B_iter, list<vector<list<Btable>>>::iterator iend){
    list<vector<list<Btable>>>::iterator i=B_iter;
    i++;
    while(i!=iend) // 在一次循环内，B_iter是首个节点，i是当前正在合并进首节点的节点，iend是尾节点。
    {
        
        vector<list<Btable>> Mul_result(B_iter->size()+i->size()-1);//两个节点处理之后的结果存放处
        for(int x=1;x<B_iter->size();x++)//x和y分别遍历两个节点的时间可能性
        {
            if((*B_iter)[x].size()==0) continue;//如果格子为空
            for(int y=1;y<i->size();y++)
            {
                if((*i)[y].size()==0) continue;//如果格子为空
                list<Btable> L=Circle_Multiply((*B_iter)[x],(*i)[y]);
                    Mul_result[x+y].insert(Mul_result[x+y].end(),L.begin(),L.end());
            }
        }
        // *B_iter=Mul_result;
        B_iter->swap(Mul_result);
        for(int x=1;x<B_iter->size()-1;x++)
        {
            
            BitMapRemoval((*B_iter)[x]);
            Newremoval((*B_iter)[x]);
            // (*B_iter)[x+1].insert((*B_iter)[x+1].end(),(*B_iter)[x].begin(),(*B_iter)[x].end());
        }
        BitMapRemoval((*B_iter)[B_iter->size()-1]);
        Newremoval((*B_iter)[B_iter->size()-1]);
        auto index_delete=i;
        i++;
        B.erase(index_delete);     
    }
    pthread_exit(NULL);
    return;
}
void Path_Assign(int stride) {  
    list<vector<list<Btable>>>::iterator B_iter=B.begin();//从头开始
    int pthread_count=B.size()%stride==0?B.size()/stride:B.size()/stride+1;
    int pthread_i=0;
    thread threads[pthread_count];
    list<vector<list<Btable>>>::iterator iend=B_iter;
    // auto start_time = std::chrono::high_resolution_clock::now();
    while(B_iter!=B.end())//只要迭代器还没到达最后，则继续
    {   
        for(int t=0;t<stride;t++)
        {
            iend++;
            if(iend==B.end()) break;//假设i从节点0开始，则iend指向节点3，则我们要处理的是0、1、2，然后让B_iter直接跳到iend
        }
        threads[pthread_i++]=thread(thread_function,B_iter,iend);
        B_iter=iend;
    }
    for (int i = 0; i < pthread_count; i++) {
        threads[i].join();
    }
    // auto end_time = std::chrono::high_resolution_clock::now();
    // auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // cout << "parallel part executing time" << duration.count() << "ms\n";
    return;
}
void Path_Wrong() {  
    list<vector<list<Btable>>>::iterator B_iter=B.begin();//从头开始
    list<vector<list<Btable>>>::iterator i=B_iter;
    i++;
    int count=0;
    while(i!=B.end())
    {
        
        vector<list<Btable>> Mul_result(B_iter->size()+i->size()-1);//两个节点处理之后的结果存放处
        // cout<<chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count()<<endl;
        for(int x=1;x<B_iter->size();x++)//x和y分别遍历两个节点的时间可能性
        {
            if((*B_iter)[x].size()==0) continue;//如果格子为空
            for(int y=1;y<i->size();y++)
            {
                if((*i)[y].size()==0) continue;//如果格子为空
                // chrono::high_resolution_clock::time_point insert_start=chrono::high_resolution_clock::now();
                list<Btable> L=Circle_Multiply_Without_Bitmap((*B_iter)[x],(*i)[y]);
                count++;
                // cout<<chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - insert_start).count()<<endl;
                Mul_result[x+y].insert(Mul_result[x+y].end(),L.begin(),L.end());
                
            }
        }
        B_iter->swap(Mul_result);
        for(int x=1;x<B_iter->size()-1;x++)
        {
            removal((*B_iter)[x]);
            if(B.size()==2) (*B_iter)[x+1].insert((*B_iter)[x+1].end(),(*B_iter)[x].begin(),(*B_iter)[x].end());
        }
        removal((*B_iter)[B_iter->size()-1]);
        auto index_delete=i;
        i++;
        B.erase(index_delete); 
        // cout<<chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count()<<endl;    
        
    }
    // printf("count=%d\n",count);
    return;
}
int main(int argc, char* argv[])
{
    
    int stride=stoi(argv[1]);
    inputData();


    auto start_time = std::chrono::high_resolution_clock::now();
    creatTableB();
    Path_Assign(stride);
    
    Path_Wrong();
    

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    cout << "program executing time" << duration.count() << "ms" << std::endl;
    // cout <<duration.count();

    
    
    //输出时间：(概率，cost)
    // for(auto &i:B)
    // {

    //     for(int x=1;x<i.size();x++)
    //     {
    //         if(i[x].size()==0) continue;
    //         printf("time=%d: ",x);
    //         for(auto &k:i[x])
    //         {
    //             printf("(%f,%d)",k.prob,k.cost);
    //         }
    //         cout<<'\n';

    //     }

    // }


    // string X="[",Y="[",Z="[";//用X,Y,Z存放坐标值
    // for(auto &i:B)
    // {

    //     for(int x=1;x<i.size();x++)
    //     {
    //         if(i[x].size()==0) continue;
    //         for(auto &k:i[x])
    //         {
    //             X+=to_string(x)+",";
    //             Y+=to_string(k.cost)+",";
    //             Z+=to_string(k.prob)+",";
    //         }
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
}