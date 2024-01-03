#include <iostream>
#include<assert.h>
#include <vector>
#include<list>
using namespace std;
#define NodeNumber 70
#define FUtypeNumber 2
#define EntryNumber 2
#define GAMA 1e-30
struct TimProCos {
    int time;
    double probability;
    int cost;
    int FU_type;
    int entry_id;
    TimProCos(int t=0, double p=0, int c=0, int f=0, int e=0){
        time = t;
        probability = p;
        cost = c;
        FU_type = f;
        entry_id = e;
    }
};
TimProCos nodes_data[NodeNumber][FUtypeNumber][EntryNumber];

struct Quadruple
{
    int cost;
    double prob;
    vector<unsigned int> bit_string;
    vector<int> assignments;
    Quadruple(){
        bit_string.push_back(0);
    }

    void Initial_Qua(int FUtype, unsigned int entry_id)
    { 
        assignments.push_back(FUtype);
        SetBit(0,entry_id);
    }
    bool SetBit(unsigned int n)
    {
        while(n/32+1>bit_string.size())
        {
            bit_string.push_back(0);
        }
        bit_string[n/32]|=1<<n%32;
        return true;
    }
    bool SetBit(unsigned int start, unsigned int end){
        while(end/32+1>bit_string.size())
        {
            bit_string.push_back(0);
        }
        for(int i=start;i<=end;i++)
            bit_string[i/32]|=1<<i%32;
        return true;
    }
    bool ResetBit(unsigned int n){
        // assert(n/32<bit_string.size());
        bit_string[n/32]&=~(1<<n%32);
        return true;
    }

    void UnionTwoBitString(vector<unsigned int> &r_bit_string,vector<int>& nodes)
    {
        unsigned int length=r_bit_string.size()*32;
        for(int i=0;i<length;i++)
        {
            if(!(bit_string[i/32]&1<<i%32)&&(r_bit_string[i/32]&1<<i%32))
            {
                prob+=Add_prob(i,nodes);
                SetBit(i);
            }
        }
    }
    bool Index2EventSequence(int n,vector<int>& event_sequence)
    {
        int length=event_sequence.size();
        for(int i=0;i<length;i++)
        {
            event_sequence[length-i-1]=n%EntryNumber;
            n/=EntryNumber;
        }
        return true;
    }
    unsigned int EventSequence2Index(vector<int>& event_sequence)
    {
        unsigned int res=0;
        unsigned int base=1;
        unsigned int length=event_sequence.size();
        for(unsigned int i=0;i<length;i++)
        {
            res+=base*event_sequence[length-i-1];
            base*=EntryNumber;
        }
        return res;
    }
    double Add_prob(int n,vector<int>& nodes)
    {
        double result=1.0;
        vector<int> event_sequence(nodes.size());
        Index2EventSequence(n,event_sequence);
        for(int i=0;i<nodes.size();i++)
        {
            result*=nodes_data[nodes[i]][assignments[i]][event_sequence[i]].probability;
        }
        return result;
    }
    Quadruple UpdateWithBitString(Quadruple qua_in_Btable,int nodes_number){
        Quadruple result;
        assert(result.assignments.size()==0);
        result.cost=cost+qua_in_Btable.cost;
        result.prob=prob*qua_in_Btable.prob;
        result.assignments.insert(result.assignments.end(),assignments.begin(),assignments.end());
        result.assignments.insert(result.assignments.end(),qua_in_Btable.assignments.begin(),qua_in_Btable.assignments.end());

        vector<unsigned int> &r_Bstring=qua_in_Btable.bit_string;
        unsigned int length=bit_string.size()*32;
        for(unsigned int i=0;i<length;i++){
            if(bit_string[i/32]&1<<i%32)
            {
                vector<int> old_event_sequence(nodes_number); //caution wfw
                Index2EventSequence(i,old_event_sequence);
                for(unsigned int idx=0;idx<EntryNumber;idx++)
                {
                    if(r_Bstring[0]&1<<idx){
                        vector<int> new_event_sequence(old_event_sequence.begin(),old_event_sequence.end());
                        new_event_sequence.push_back(idx);
                        result.SetBit(EventSequence2Index(new_event_sequence));
                    }
                }
            }
        }
        return result;
    }
    Quadruple UpdateWithoutBitString(Quadruple qua_in_Btable){
        Quadruple result;
        assert(result.assignments.size()==0);
        result.cost=cost+qua_in_Btable.cost;
        result.prob=prob*qua_in_Btable.prob;

        result.assignments.insert(result.assignments.end(),assignments.begin(),assignments.end());
        result.assignments.insert(result.assignments.end(),qua_in_Btable.assignments.begin(),qua_in_Btable.assignments.end());
        return result; 
    }
};
bool compareCost(const Quadruple &a, const Quadruple &b)
{
    return a.cost < b.cost;
}
struct TableRow
{
    vector<list<Quadruple>> row;
    unsigned int min_time;
    unsigned int max_time;
    vector<int> nodes;
    void removal(unsigned int &time)
    {
        list<Quadruple>& L=row[time];
        for(list<Quadruple>::iterator i = L.begin(); i != --L.end();) 
        {
            list<Quadruple>::iterator left=i,right = i;
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
    void Redundant_Without_Bstring(unsigned int &time)
    {
        row[time].sort(compareCost);
        removal(time);
    }
    void Merging(unsigned int time)
    {
        list<Quadruple>& L=row[time];
        for(list<Quadruple>::iterator i = L.begin(); i !=L.end();i++)
        for(list<Quadruple>::iterator j = i; j !=L.end();j++)
        {
            if(i->cost==-1) break;
            if(i->cost!=j->cost) break;
            if(i==j) continue;
            if(j->cost==-1) continue;
            assert(i->assignments.size()==j->assignments.size());
            int x=0;
            for(;x<i->assignments.size();x++)
            {
                if(i->assignments[x]!=j->assignments[x]) break;
            }
            if(x!=i->assignments.size()) continue;

            if(i->bit_string.size()>j->bit_string.size())
            {
                i->UnionTwoBitString(j->bit_string,nodes);
                if(i->prob>1) i->prob=1.0;
                j->cost=-1;
            }
            else
            {
                j->UnionTwoBitString(i->bit_string,nodes);
                if(j->prob>1) j->prob=1.0;
                i->cost=-1;
            }

        }
        for(list<Quadruple>::iterator i = L.begin(); i !=L.end();)
        {
            list<Quadruple>::iterator j=i;
            j++;
            if(i->cost==-1) {L.erase(i);}
            i=j;
        }
        return;
    }   
    void Redundant_With_Bstring(unsigned int &time)
    {
        row[time].sort(compareCost);
        Merging(time);
        removal(time);
    }
    list<Quadruple> Circle_Multiply_Without_Bstring(list<Quadruple>& L1,list<Quadruple>& L2)
    {
        list<Quadruple> result;
        for(list<Quadruple>::iterator i=L1.begin();i!=L1.end();i++)
            for(list<Quadruple>::iterator j=L2.begin();j!=L2.end();j++){
                result.push_back(i->UpdateWithoutBitString(*j));
            }
        return result;
    }
    list<Quadruple> Circle_Multiply_With_Bstring(list<Quadruple>& L1,list<Quadruple>& L2,int nodes_number)
    {
        list<Quadruple> result;
        for(list<Quadruple>::iterator i=L1.begin();i!=L1.end();i++)
            for(list<Quadruple>::iterator j=L2.begin();j!=L2.end();j++){
                result.push_back(i->UpdateWithBitString(*j,nodes_number));
            }
        return result;
    }
    void Update_Row_Without_Bstring(TableRow r_table_row)
    {
        vector<list<Quadruple>> new_row(max_time+r_table_row.max_time+1);
        for(unsigned int i=min_time;i<=max_time;i++)
        {
            for(unsigned int j=r_table_row.min_time;j<=r_table_row.max_time;j++)
            {
                list<Quadruple> L=Circle_Multiply_Without_Bstring(row[i],r_table_row.row[j]);
                new_row[i+j].insert(new_row[i+j].end(),L.begin(),L.end());
            }
        }
        row.swap(new_row);
        min_time+=r_table_row.min_time;
        max_time+=r_table_row.max_time;
        nodes.insert(nodes.end(),r_table_row.nodes.begin(),r_table_row.nodes.end());
        // appending and applying redundant
        Redundant_Without_Bstring(min_time);
        for(unsigned int i=min_time+1;i<=max_time;i++)
        {
            row[i].insert(row[i].end(),row[i-1].begin(),row[i-1].end());
            Redundant_Without_Bstring(i);
        }
        return;
    }
    void Update_Row_With_Bstring(TableRow r_table_row)
    {
        vector<list<Quadruple>> new_row(max_time+r_table_row.max_time+1);
        for(unsigned int i=min_time;i<=max_time;i++)
        {
            for(unsigned int j=r_table_row.min_time;j<=r_table_row.max_time;j++)
            {
                list<Quadruple> L=Circle_Multiply_With_Bstring(row[i],r_table_row.row[j],nodes.size());
                new_row[i+j].insert(new_row[i+j].end(),L.begin(),L.end());
            }
        }
        row.swap(new_row);
        min_time+=r_table_row.min_time;
        max_time+=r_table_row.max_time;
        nodes.insert(nodes.end(),r_table_row.nodes.begin(),r_table_row.nodes.end());
        // appending and applying redundant
        Redundant_With_Bstring(min_time);
        for(unsigned int i=min_time+1;i<=max_time;i++)
        {
            row[i].insert(row[i].end(),row[i-1].begin(),row[i-1].end());
            Redundant_With_Bstring(i);
        }
        return;
    }
    void Union_2_rows(TableRow r_table_row)
    {
        vector<list<Quadruple>> new_row(max(max_time,r_table_row.max_time)+1);
        unsigned int i;
        for(i=max(min_time,r_table_row.min_time);i<=min(max_time,r_table_row.max_time);i++)
        {
            list<Quadruple> L=Circle_Multiply_Without_Bstring(row[i],r_table_row.row[i]);
            new_row[i].insert(new_row[i].end(),L.begin(),L.end());
        }
        if(max_time>r_table_row.max_time)
        {
            for(;i<=max_time;i++)
            {
                list<Quadruple> L=Circle_Multiply_Without_Bstring(row[i],r_table_row.row[r_table_row.max_time]);
                new_row[i].insert(new_row[i].end(),L.begin(),L.end());
            }
        }
        else if(max_time<r_table_row.max_time)
        {
            for(;i<=r_table_row.max_time;i++)
            {
                list<Quadruple> L=Circle_Multiply_Without_Bstring(row[max_time],r_table_row.row[i]);
                new_row[i].insert(new_row[i].end(),L.begin(),L.end());
            }
        }
        row.swap(new_row);
        min_time=max(min_time,r_table_row.min_time);
        max_time=max(max_time,r_table_row.max_time);
        nodes.insert(nodes.end(),r_table_row.nodes.begin(),r_table_row.nodes.end());
        // appending and applying redundant 
        Redundant_Without_Bstring(min_time);
        for(unsigned int i=min_time+1;i<=max_time;i++)
        {
            row[i].insert(row[i].end(),row[i-1].begin(),row[i-1].end());
            Redundant_Without_Bstring(i);
        }
        return;
    }
};

vector<TableRow> B;

int entry_number[NodeNumber][FUtypeNumber]={0};
int FU_type[NodeNumber]={0};
int node_num;
int count_equal = 0;