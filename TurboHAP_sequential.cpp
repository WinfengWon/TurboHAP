/* 
1.Blocking
2.Sort by costs
3.without intermediate appending
4.Sequential execution
*/
#include <iostream>
#include <vector>
#include<assert.h>
#include <algorithm>
#include <list>
#include <chrono>
#include<math.h>
#include "TurboHAP.h"
using namespace std;
void Input_data() {

    cin >> Node_num;
    B.resize(Node_num);
    for (int i = 0; i < Node_num; i++) {
        int FU_num; cin >> FU_num; // Number of FU types of the current node
        for (int j = 0; j < FU_num; j++) {
            int entry_num; cin >> entry_num; // Number of entries of the current FU type
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
bool Compare_cost(const Btable &a, const Btable &b)
{
    return a.cost < b.cost;
}
void Merging(list<Btable>& L)
{
    L.sort(Compare_cost);
    for(list<Btable>::iterator i = L.begin(); i !=L.end();i++)
    for(list<Btable>::iterator j = i; j !=L.end();j++)
    {
        if(i->cost==-1) break;
        if(i->cost!=j->cost) break;
        if(i==j) continue;
        if(j->cost==-1) continue;
        assert(i->FUtypeArr.size()==j->FUtypeArr.size());

        int x=0;
        for(;x<i->FUtypeArr.size();x++) // Determine if the 2 assignments are equal
        {
            if(i->FUtypeArr[x]!=j->FUtypeArr[x]) break;
        }
        if(x!=i->FUtypeArr.size()) continue;

        if(i->bit_string.size()>j->bit_string.size()) // Select the one with longer bit_string as foundation
        {
            i->UnionTwoE(j->bit_string);
            if(i->prob>1) i->prob=1.0;
            j->cost=-1;
        }
        else
        {
            j->UnionTwoE(i->bit_string);
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
void Redundant_without_merging(list<Btable>& L)
{
    L.sort(Compare_cost);
    Newremoval(L);
}
void Redundant_with_merging(list<Btable>& L)
{
    Merging(L);
    Newremoval(L);
}
void Creat_Btable() {
    list<vector<list<Btable>>>::iterator B_iter=B.begin();
    for (int i = 0; i < Node_num; i++,B_iter++) 
    {       
        B_iter->resize(MaxTimeOfB[i]+1);
        for (int j = 0; j<FUtypeNumber; j++)
        {
            double accumulate=0;
            for (int k = 0; k < EntryNumOfData[i][j]; k++) {
                Btable a;
                a.cost = Data[i][j][k].cost;
                a.time = Data[i][j][k].time;
                a.FUtype=j;
                a.entry=k;
                accumulate+=Data[i][j][k].probability;
                a.prob=accumulate;
                a.initial(j,k,i); // Initialize the quadruple
                (*B_iter)[a.time].push_back(a); // Insert quadruple in linked list
            }
        }

        for(int t=2;t<=MaxTimeOfB[i];t++)
        {
            (*B_iter)[t].insert((*B_iter)[t].end(),(*B_iter)[t-1].begin(),(*B_iter)[t-1].end()); // Appending
            Redundant_without_merging((*B_iter)[t]); // Apply redundant_without_merging for B table
        }

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
list<Btable> Circle_Multiply(list<Btable>& L1,list<Btable>& L2){
    list<Btable> L;
    for(list<Btable>::iterator i=L1.begin();i!=L1.end();i++)
        for(list<Btable>::iterator j=L2.begin();j!=L2.end();j++){
            L.push_back(i->circle_multiply(*j));
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

void Sequential_blocking(int stride){  
    list<vector<list<Btable>>>::iterator B_iter=B.begin();
    auto start_time = std::chrono::high_resolution_clock::now();
    while(B_iter!=B.end())
    {
        list<vector<list<Btable>>>::iterator i=B_iter,iend=B_iter;
        for(int t=0;t<stride;t++)
        {
            iend++;
            if(iend==B.end()) break;
        }
        i++;
        while(i!=iend)
        {
            
            vector<list<Btable>> Mul_result(B_iter->size()+i->size()-1);
            for(int x=1;x<B_iter->size();x++)
            {
                if((*B_iter)[x].size()==0) continue;
                for(int y=1;y<i->size();y++)
                {
                    if((*i)[y].size()==0) continue;
                    list<Btable> L=Circle_Multiply((*B_iter)[x],(*i)[y]);
                        Mul_result[x+y].insert(Mul_result[x+y].end(),L.begin(),L.end());
                }
            }
            // *B_iter=Mul_result;
            B_iter->swap(Mul_result);
            for(int x=1;x<B_iter->size()-1;x++)
            {    
                Redundant_with_merging((*B_iter)[x]);
                // (*B_iter)[x+1].insert((*B_iter)[x+1].end(),(*B_iter)[x].begin(),(*B_iter)[x].end());
            }
            Redundant_with_merging((*B_iter)[B_iter->size()-1]);
            auto index_delete=i;
            i++;
            B.erase(index_delete);     
        }
        B_iter++;

    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    cout << "parallel part executing time" << duration.count() << "ms\n";
    return;
}
void Merge_blocks(){  
    list<vector<list<Btable>>>::iterator B_iter=B.begin();
    list<vector<list<Btable>>>::iterator i=B_iter;
    i++;
    int count=0;
    while(i!=B.end())
    {
        
        vector<list<Btable>> Mul_result(B_iter->size()+i->size()-1);
        // cout<<chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count()<<endl;
        for(int x=1;x<B_iter->size();x++)
        {
            if((*B_iter)[x].size()==0) continue;
            for(int y=1;y<i->size();y++)
            {
                if((*i)[y].size()==0) continue;
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
            Redundant_without_merging((*B_iter)[x]);
            if(B.size()==2) (*B_iter)[x+1].insert((*B_iter)[x+1].end(),(*B_iter)[x].begin(),(*B_iter)[x].end());
        }
        Redundant_without_merging((*B_iter)[B_iter->size()-1]);
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
    Input_data();
    auto start_time = std::chrono::high_resolution_clock::now();
    Creat_Btable();
    Sequential_blocking(stride);
    Merge_blocks();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    cout << "program executing time" << duration.count() << "ms" << std::endl;
    // cout <<duration.count();

    
    
    //print time：(probability，cost)
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


// string X="[",Y="[",Z="[";//X, Y, Z are used to store time, cost, probability, respectively.
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