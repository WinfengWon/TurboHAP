/* 
1.Blocking
2.Sort by costs
3.Without intermediate appending
4.Tree of transformer
*/
#include <algorithm>
#include <pthread.h>
#include <thread>
#include <chrono>
#include<math.h>
#include "TurboHAP_tree.h"
using namespace std;
#define MAX_INT 2147483647
void InputData() {
    cin >> node_num;
    B.resize(node_num);
    for (int i = 0; i < node_num; i++) {
        int FU_num,min_time=MAX_INT,max_time=0; 
        cin >> FU_num;
        FU_type[i]=FU_num;
        for (int j = 0; j < FU_num; j++) {
            int entry_num;
            cin >> entry_num;
            entry_number[i][j]=entry_num;
            for (int k = 0; k < entry_num; k++) {
                int time, cost;
                double probability;
                cin >> time >> probability >> cost;
                nodes_data[i][j][k].time = time;
                nodes_data[i][j][k].probability = probability;
                nodes_data[i][j][k].cost = cost;
                nodes_data[i][j][k].FU_type=j;
                nodes_data[i][j][k].entry_id=k;
                max_time=time>max_time?time:max_time;
                min_time=time<min_time?time:min_time;
            }
        }
        B[i].max_time=max_time;
        B[i].min_time=min_time;
        B[i].nodes.push_back(i);
        B[i].row.resize(max_time+1);
    }
}
void Initialize_B_table()
{
    for (int i = 0; i < node_num; i++) 
    {
        for (int j = 0; j < FU_type[i]; j++) 
        {
            double probability=0.0;
            for (int k = 0; k < entry_number[i][j]; k++) 
            {
                TimProCos &node_data=nodes_data[i][j][k];
                probability+=node_data.probability;
                if(k==entry_number[i][j]-1) 
                    probability=1.0;
                Quadruple q;
                q.cost=node_data.cost;
                q.prob=probability;
                q.Initial_Qua(j,k);
                B[i].row[node_data.time].push_back(q);
            }
        }
        B[i].Redundant_Without_Bstring(B[i].min_time);
        for(unsigned int t=B[i].min_time+1;t<=B[i].max_time;t++)
        {
            B[i].row[t].insert(B[i].row[t].end(),B[i].row[t-1].begin(),B[i].row[t-1].end());
            B[i].Redundant_Without_Bstring(t);
        }
    }
}
int main(int argc, char* argv[])
{
    InputData();
    Initialize_B_table();
    
    // TurboHAP for transformer graph of 20 nodes
    for(int i=1;i<=6;i++)
    {
        B[0].Update_Row_With_Bstring(B[i]);
    }
    for(int i=8;i<=12;i++)
    {
        B[7].Update_Row_With_Bstring(B[i]);
    }
    B[0].Union_2_rows(B[7]);
    for(int i=14;i<=19;i++)
    {
        B[13].Update_Row_With_Bstring(B[i]);
    }
    B[0].Update_Row_Without_Bstring(B[13]);

    // DAG_Heu for transformer graph of 20 nodes
    // for(int i=1;i<=6;i++)
    // {
    //     B[0].Update_Row_Without_Bstring(B[i]);
    // }
    // for(int i=8;i<=12;i++)
    // {
    //     B[7].Update_Row_Without_Bstring(B[i]);
    // }
    // B[0].Union_2_rows(B[7]);
    // for(int i=14;i<=19;i++)
    // {
    //     B[13].Update_Row_Without_Bstring(B[i]);
    // }
    // B[0].Update_Row_Without_Bstring(B[13]);

    for(int t=B[0].min_time;t<=B[0].max_time;t++)
    {
        printf("time=%d: ",t);
        for(list<Quadruple>::iterator it=B[0].row[t].begin();it!=B[0].row[t].end();it++)
        printf("(%f,%d)",it->prob,it->cost);
        printf("\n");
    }

    // auto start_time = std::chrono::high_resolution_clock::now();
    

    // auto end_time = std::chrono::high_resolution_clock::now();

    // auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // cout << "program executing time" << duration.count() << "ms" << std::endl;

    
    


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
    return 0;
}