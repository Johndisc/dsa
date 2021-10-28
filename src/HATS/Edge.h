//
// Created by CGCL on 2021/10/28.
//

#ifndef ZSIM_EDGE_H
#define ZSIM_EDGE_H

struct Edge {
    int u;
    int v;
    Edge() {};
    Edge(int _u, int _v):u(_u),v(_v) {};
};

#endif //ZSIM_EDGE_H
