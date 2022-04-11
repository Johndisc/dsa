//
// Created by CGCL on 2021/10/28.
//

#ifndef ZSIM_EDGE_H
#define ZSIM_EDGE_H
#include <vector>

using namespace std;

struct Edge {
    int u;
    int v;
    int data;
    Edge() {};
    Edge(int _u, int _v):u(_u),v(_v) {};
    Edge(int _u, int _v, int _data):u(_u),v(_v),data(_data) {};
};

class HATS {
public:
    int hid{0};

    virtual void
    configure(vector<int> *_offset, vector<int> *_neighbor, vector<bool> *_active, vector<int> *_weight,
              int *_vertex_data, bool _isPush, int _start_v, int _end_v, int _hid) = 0;

    virtual void fetchEdges(Edge &edge) = 0;

    virtual void start() = 0;
};
#endif //ZSIM_EDGE_H
