# TurboHAP
The source code of paper "TurboHAP: A More Confident Method for Heterogeneous Assignment with  Probability While Satisfying Hard/Soft Timing Constraints"

# Note
We prove the effectiveness of TurboHAP with path structures.

However, we cannot provide the source code of DAG structure, since the **probabilities of common nodes** are calculated multiple times. This **phenomenon** is described in  DAG_Heu, and it is found that **DAG_Heu cannot solve the problem of common node**.
We believe TurboHAP can outperforms DAG_Heu in DAG structure, since we have bit strings trace the redundant probabilities of common nodes.

# Input form
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
