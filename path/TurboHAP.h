#include <iostream>
#include<assert.h>
#include <vector>
using namespace std;
#define NodeNumber 70 //Maximum number of nodes
#define FUtypeNumber 2 //Maximum number of FU types
#define EntryNumber 2 //Maximum number of entries within a FU type of a node
#define TimeColum 20 //Maximum allowed time for each node
#define DebugTableB false //Set whether to output B table
#define GAMA 1e-30

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
TimProCos Data[NodeNumber][FUtypeNumber][EntryNumber];//Store input data

struct Event{
    vector<unsigned int> bit_string;
    double prob;
    unsigned int start; //Record the starting node of current event sequence
    unsigned int end;   //Record the ending node of current event sequence
    vector<int> FUtypeArr; //Record assignment
    Event(){
        bit_string.push_back(0);
        prob=1.0;
    }

    /*`e` represents event. Specifically, 
    `e==0` represents the last bit of bit_string set as `1`;
    `e==1` represents the last 2 bit of bit_string set as `1`;*/
    void initial(unsigned int FUtype,unsigned int e,unsigned int s)
    {
        SetBit(0,e);
        start=s;
        end=s;
        FUtypeArr.push_back(FUtype);
    }

    //Set the n-th bit of bit_string as `1`, `n` starts from `0`
    bool SetBit(unsigned int n){
        while(n/32+1>bit_string.size())
        {
            bit_string.push_back(0);
        }
        bit_string[n/32]=bit_string[n/32]|1<<n%32;
        return true;
    }
    //Set the n-th bit of bit_string as `0`
    bool ResetBit(unsigned int n){
        // assert(n/32<bit_string.size());
        bit_string[n/32]=bit_string[n/32]&~(1<<n%32);
        return true;
    }
    //Set all bits from index `l` to index `r` as `1`, including `l` and `r`
    bool SetBit(unsigned int l,unsigned int r){
        while(r/32+1>bit_string.size())
            bit_string.push_back(0);
        for(unsigned int i=l;i<=r;i++)
            SetBit(i);
        return true;
    }
    void Update(int FUtype,unsigned int flag){ //Update bit_string and assignment
        unsigned int length=bit_string.size()*32;
        Event new_bit_string;
        for(unsigned int i=0;i<length;i++){
            if(bit_string[i/32]&1<<i%32){
                if(flag==1){
                    unsigned int n=2*i;
                    new_bit_string.SetBit(n);
                    new_bit_string.SetBit(n+1);
                }
                else{
                    unsigned int n=2*i; 
                    new_bit_string.SetBit(n);
                }
            }
        }
        FUtypeArr.push_back(FUtype);
        end++;
        bit_string=move(new_bit_string.bit_string);
        return;
    }

};
struct Btable:Event
{
    int cost;
    int time;
    int FUtype;
    int entry;
    Btable(){}
    void UnionTwoE(vector<unsigned int> &rbit_string) //Union 2 bit_string
    {
        unsigned int length=rbit_string.size()*32;
        for(int i=0;i<length;i++)
        {
            if(rbit_string[i/32]&1<<i%32&&!(bit_string[i/32]&1<<i%32))
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


//Define global variables
list<vector<list<Btable>>> B; //Use B replace the B and D of DAG_Heu
int EntryNumOfData[NodeNumber][FUtypeNumber]={0}; //Record the number of entries within each FU type of each node
int MaxTimeOfB[NodeNumber]={0};
int Node_num; //Number of nodes of input 
int count_equal = 0;