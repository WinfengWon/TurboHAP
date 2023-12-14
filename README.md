# TurboHAP
The source code of paper "TurboHAP: A More Confident Method for Heterogeneous Assignment with  Probability While Satisfying Hard/Soft Timing Constraints"

# Note
We prove the effectiveness of TurboHAP with both path and tree structures (specific graphs are shown following). 

However, we cannot provide the source code of DAG structure, since the **probabilities of common nodes** are calculated multiple times. This **phenomenon** is described in  DAG_Heu, and it is found that **DAG_Heu cannot solve the problem of common node**.

We believe TurboHAP can outperforms DAG_Heu in DAG structure, since we have bit strings trace the redundant probabilities of common nodes.

# Input form
## Path
For example, the default input is formed as follows:

Node number  
the number of FU types for node 0  
FU type 1  
entry0  
entry1  
FU type 2  
entry0  
entry1  
the number of FU types for node 1  
FU type 1  
entry0  
entry1  
FU type 2  
entry0  
entry1  

## Tree
We use the popular transformer model as the graph structure, which is actually tree structure.  
The node data is the same as that in path.  
The graph of transformer is as follows.

<img src="https://github.com/WinfengWon/TurboHAP/assets/153063846/13ebe7ef-c611-4a2d-81f2-2a3c741dcf1c" alt="图片描述" width="300">

The image is cited from "[Attention Is All You Need](https://proceedings.neurips.cc/paper_files/paper/2017/file/3f5ee243547dee91fbd053c1c4a845aa-Paper.pdf)"
