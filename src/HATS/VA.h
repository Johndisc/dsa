//
// Created by CGCL on 2021/9/28.
//

#ifndef ZSIM_VA_H
#define ZSIM_VA_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include "shared_memory.h"

#define MAX_DEPTH 10
#define THREAD_NUM 10

using namespace std;

mutex fifo_mutex;

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
    int scan()
    {
        for (int i = current_vid; i < last_vid; i++) {
            if ((*active_bits)[i]) {
                (*active_bits)[i] = false;
                current_vid++;
                return i;
            }
        }
        return -1;
    }

    void fetch_offsets(int vid, int &start_offset, int &end_offset)
    {
        start_offset = offset[vid];
        end_offset = offset[vid + 1];
    }

    vector<Edge> fetch_neighbors(int vid, int start_offset, int end_offset)
    {
        vector <Edge> neighbors;
        for (int i = start_offset; i < end_offset; ++i) {
            while (this->FIFO.size() > MAX_DEPTH)
                this_thread::yield();               //fifo满时HATS停止
            lock_guard<mutex> lock(fifo_mutex);
            FIFO.push(Edge(vid, neighbor[i]));
            neighbors.emplace_back(vid, i);
        }
        return neighbors;
    }

    T prefetch(int vid)
    {
        return vertex_data[vid];
    }

    void start()
    {
        int vid, start_offset, end_offset;
        vector <Edge> edges;
        while (current_vid < last_vid) {
            vid = scan();
            fetch_offsets(vid, start_offset, end_offset);
            edges = fetch_neighbors(vid, start_offset, end_offset);
        }
        current_vid++;
    }

public:
    VA(){};
    ~VA()= default;

    void hats_configure(vector<int> _offset, vector<int> _neighbor, vector<bool> *_active, vector<T> _vertex_data, bool _isPush,
                        int _start_v, int _end_v)
    {
        offset = _offset;
        neighbor = _neighbor;
        active_bits = _active;
        vertex_data = _vertex_data;
        isPush = _isPush;
        current_vid = _start_v;
        last_vid = _end_v;
        start();
        //thread
    }

    void hats_fetch_edges(Edge &edge)
    {
        while (this->FIFO.empty() && current_vid <= last_vid)
            this_thread::yield();           //fifo为空时fetch停止
        lock_guard<mutex> lock(fifo_mutex);
        if (!FIFO.empty())
        {
            edge = FIFO.front();
            FIFO.pop();
        }
        else
            edge = Edge(-1, -1);
    }
};

void configure(vector<int> _offset, vector<int> _neighbor, vector<bool> *_active, bool _isPush, int _start_v, int _end_v)
{
    unsigned data[9];
    data[0] = (int) &_offset;
    data[1] = sizeof(_offset);
    data[2] = (int) &_neighbor;
    data[3] = sizeof(_neighbor);
    data[4] = (int) &_active;
    data[5] = sizeof(_active);
    data[6] = (int) _isPush;
    data[7] = _start_v;
    data[8] = _end_v;

    void *addr = getSharedMemAddr();
    memcpy(addr, data, sizeof(data));
}

#endif //ZSIM_VA_H
