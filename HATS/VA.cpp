//
// Created by CGCL on 2021/9/28.
//

#include "VA.h"

using namespace std;

mutex active_mutex;
mutex fifo_mutex;

template<typename T>
VA<T>::VA(){}

template <typename T>
VA<T>::~VA<T>() = default;

template <typename T>
int VA<T>::scan() {
    for (int i = current_vid; i < last_vid; i++) {
        if ((*active_bits)[i]) {
            lock_guard<mutex> lock(active_mutex);
            (*active_bits)[i] = false;
            return i;
        }
    }
}

template<typename T>
void VA<T>::fetch_offsets(int vid, int &start_offset, int &end_offset) {
    start_offset = neighbor[offset[vid]];
    end_offset = neighbor[offset[vid + 1] - 1];
}

template<typename T>
vector<Edge> VA<T>::fetch_neighbors(int vid, int start_offset, int end_offset) {
    vector <Edge> neighbors;
    for (int i = start_offset; i < end_offset; ++i) {
        while (this->FIFO.size() > MAX_DEPTH)
            this_thread::yield();
        lock_guard<mutex> lock(fifo_mutex);
        FIFO.push(Edge(vid, i));
        neighbors.emplace_back(vid, i);
    }
    return neighbors;
}

template <typename T>
T VA<T>::prefetch(int vid) {
    return vertex_data[vid];
}

template <typename T>
void VA<T>::start() {
    int vid, start_offset, end_offset;
    vector <Edge> edges;
    vid = scan();
    fetch_offsets(vid, start_offset, end_offset);
    edges = fetch_neighbors(vid, start_offset, end_offset);
}

template <typename T>
void VA<T>::hats_configure(vector<int> _offset, vector<int> _neighbor, vector<bool> *_active, vector<T> _vertex_data, bool _isPush,
                           int _start_v, int _end_v) {
    offset = _offset;
    neighbor = _neighbor;
    active_bits = _active;
    vertex_data = _vertex_data;
    isPush = _isPush;
    current_vid = _start_v;
    last_vid = _end_v;
    //thread
}

template <typename T>
Edge VA<T>::hats_fetch_edges() {
    Edge edge = FIFO.front();
    FIFO.pop();
    return edge;
}