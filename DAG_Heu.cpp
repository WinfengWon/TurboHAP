#include <iostream>
#include <vector>
#include<list>
#include <algorithm>
#include <assert.h>
#include<math.h>
#include <chrono>
#define NodeNumber 70
#define FUtypeNumber 2 //Maximum number of FU types
#define EntryNumber 2 //Maximum number of entries within a FU type of a node
#define TimeColum 20 //Maximum allowed time for each node
#define DebugTableB true //Set whether to output B table
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
// Define global variables
list <ProbAndCostPair> D[NodeNumber][TimeColum*NodeNumber]; //Maximum allowed time for D table = TimeColum*NodeNumber
list <ProbAndCostPair> B[NodeNumber][TimeColum]; //B table, stores information about each node itself
TimProCos Data[NodeNumber][FUtypeNumber][EntryNumber];
int EntryNumOfData[NodeNumber][FUtypeNumber]={0}; //Record how many time variation within each FU type of each node has
int MaxTimeOfB[NodeNumber]={0}; //Record maximum time of each node
int Node_num; //number of nodes of input 
int count_equal = 0;

void inputData() {
    cin >> Node_num;
    for (int i = 0; i < Node_num; i++) {
        int FU_num; cin >> FU_num; //How many FU types the current node has
        for (int j = 0; j < FU_num; j++) {
            int entry_num; cin >> entry_num; //How many entries the current FU type has
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
void creatTableB() {
    for (int i = 0; i < Node_num; i++) {
        list <TimProCos> Bentry;
        TimProCos a;
        double accumulate;
        int time=0; //time is uesd to temporarily record the maximum time of current node
        for (int j = 0; j<FUtypeNumber; j++){
            accumulate=0; //For a new FU type, set accumulate as 0  
            for (int k = 0; k < EntryNumOfData[i][j]; k++) {
                a = Data[i][j][k];
                accumulate+=Data[i][j][k].probability;
                a.probability=accumulate;
                // assert(abs(a.probability - 1) < 1e-3);
                Bentry.insert(Bentry.end(),a);
            }
        }
        for(list<TimProCos>::iterator it=Bentry.begin();it!=Bentry.end();it++)
        {
            if(it->time>time)   time=it->time;
            B[i][it->time].push_back(ProbAndCostPair(it->probability,it->cost)); // insert pair into linked list
        }
        MaxTimeOfB[i]=time;    
        for(int t=2;t<=time;t++)
        {
            B[i][t].insert(B[i][t].end(),B[i][t-1].begin(),B[i][t-1].end()); //appending
            removal(B[i][t]); //applying redundant
            if(i==0)
            D[i][t]=B[i][t]; //For i==0, D(0,j)=B(0,j)
        }
        if(i==0) D[0][1]=B[0][1];
        //The following module is used to output the information of B table
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
                if(B[i][k].empty()) continue;
                if(t-k<1) break;
                if(D[i-1][t-k].empty()) continue;
                else{
                    if(min_t>t) min_t=t;
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


    // string X="[",Y="[",Z="[";//X, Y, Z are used to store time, cost, probability, respectively.
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

