//
// Created by CGCL on 2021/9/28.
//

#ifndef ZSIM_VA_H
#define ZSIM_VA_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>

#define MAX_DEPTH 10
#define THREAD_NUM 10

using namespace std;

extern mutex active_mutex;
extern mutex fifo_mutex;

struct Edge {
    int u;
    int v;
    Edge() {};
    Edge(int _u, int _v):u(_u),v(_v) {};
};

template <typename T>
class VA {
private:
    vector<int> offset;
    vector<int> neighbor;
    vector<bool> *active_bits;
    vector<T> vertex_data;
    bool isPush;

    int current_vid;
    int last_vid;

    queue<Edge> FIFO;

    thread threads[THREAD_NUM];
private:
    int scan();
    void fetch_offsets(int vid, int &start_offset, int &end_offset);
    vector<Edge> fetch_neighbors(int vid, int start_offset, int end_offset);
    T prefetch(int vid);
    void start();
public:
    VA();
    ~VA();
    void hats_configure(vector<int> _offset, vector<int> _neighbor, vector<bool> *_active, vector<T> _vertex_data, bool _isPush,
                        int _start_v, int _end_v);
    Edge hats_fetch_edges();
};


#endif //ZSIM_VA_H
