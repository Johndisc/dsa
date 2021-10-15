//
// Created by CGCL on 2021/9/17.
//

#ifndef ZSIM_BDFS_H
#define ZSIM_BDFS_H

#include <vector>
#include <queue>
# include <stack>
#include <thread>
#include <mutex>

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

struct VexInfo {
    int vex_id;
    int start_offset;
    int end_offset;
    vector<int> neighbors;
    VexInfo(int vid): vex_id(vid) {};

};

template <typename T>
class BDFS {
    private:
        vector<int> offset;
        vector<int> neighbor;
        vector<bool> *active_bits;
        vector<T> vertex_data;
        bool isPush;

        int current_vid;
        int last_vid;

        queue<Edge> FIFO;
        stack<VexInfo> vex_stack;

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

        vector<int> fetch_neighbors(int vid, int start_offset, int end_offset)
        {
            vector <int> neighbors;
            for (int i = start_offset; i < end_offset; ++i)
                neighbors.push_back(i);
            return neighbors;
        }

        void fetch_offsets(int vid, int &start_offset, int &end_offset)
        {
            start_offset = offset[vid];
            end_offset = offset[vid + 1];
        }

        void FSM()
        {
            vector<int> vneighbors;
            int vid, start_offset, end_offset;
            while (current_vid < last_vid) {
                vid = scan();
                fetch_offsets(vid, start_offset, end_offset);
                vneighbors = fetch_neighbors(vid, start_offset, end_offset);
            }
        }

    public:
        BDFS(vector<int> _offset, vector<int> _neighbor, vector <T> _vertex_data, bool _isPush, int _start_v,
             int _end_v);
        ~BDFS();
        void hats_configure(vector<int> offset, vector<int> neighbor, vector<T> vertex_data, bool isPush, int start_v, int end_v);
        void hats_fetch_edges();
};


#endif //ZSIM_BDFS_H
