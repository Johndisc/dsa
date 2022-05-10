//
// Created by CGCL on 2021/9/28.
//

#ifndef ZSIM_VO_H
#define ZSIM_VO_H

#include <cstring>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
#include "Edge.h"
#include "../zsim.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#define VO_MAX_DEPTH 10

using namespace std;

void accessL2(uint32_t tid, uint64_t address, bool isLoad);

static pthread_mutex_t vcmtx;

class VO:public HATS {
private:
    vector<int> *offset;
    vector<int> *neighbor;
    vector<bool> *active_bits;
    vector<int> *weight;
    int *vertex_data;
    bool isPush;

    int current_vid;
    int last_vid;

    queue<Edge> FIFO;
    uint32_t tid;

private:
    void scan()
    {
        for (; current_vid < last_vid; current_vid++) {
            if ((*active_bits)[current_vid]) {
                (*active_bits)[current_vid] = false;
                return;
            }
        }
    }

    void fetch_offsets(int vid, int &start_offset, int &end_offset)
    {
        start_offset = (*offset)[vid];
        end_offset = (*offset)[vid + 1];
    }

    void fetch_neighbors(int vid, int start_offset, int end_offset)
    {
        for (int i = start_offset; i < end_offset; ++i) {
            while (this->FIFO.size() > VO_MAX_DEPTH)
                this_thread::yield();               //fifo满时HATS停止
            FIFO.push(Edge(vid, (*neighbor)[i]));
        }
        prefetch(vid);
    }

    void prefetch(int vid)
    {
        accessL2(tid, (uint64_t) &vertex_data[vid], true);
    }

public:
    void start()
    {
//        int start_offset, end_offset;
//        vector <Edge> edges;
        while (true) {
            scan();
            if (current_vid == last_vid)
                break;
            FIFO.push(Edge(current_vid, 0));
//            fetch_offsets(current_vid, start_offset, end_offset);
//            fetch_neighbors(current_vid, start_offset, end_offset);
        }
    }

    VO(uint32_t _tid){ tid = _tid; };
    ~VO()= default;

    //zsim端接口
    void configure(vector<int> *_offset, vector<int> *_neighbor, vector<bool> *_active, vector<int> *_weight,
                   int *_vertex_data, bool _isPush, int _start_v, int _end_v, int _hid)
    {
        offset = _offset;
        neighbor = _neighbor;
        active_bits = _active;
        weight = _weight;
        isPush = _isPush;
        current_vid = _start_v;
        last_vid = _end_v;
        hid = _hid;
        vertex_data = _vertex_data;
    }

    //zsim端接口
    void fetchEdges(Edge &edge)
    {
        while (this->FIFO.empty() && current_vid < last_vid)
            this_thread::yield();           //fifo为空时fetch停止
        if (!FIFO.empty())
        {
            edge = FIFO.front();
            FIFO.pop();
        }
        else
            edge = Edge(-1, -1);
    }
};

//主程序端接口
inline void
hats_vo_configure(vector<int> *_offset, vector<int> *_neighbor, vector<int> *weight, int *vertex_data,
                  vector<bool> *_active, bool _isPush, int _start_v, int _end_v, int _hid) {

    int temp = (int) _isPush;

    int shmId = shmget((key_t) 1234, 100, 0666 | IPC_CREAT); //获取共享内存标志符
    void *addr = shmat(shmId, NULL, 0); //共享内存地址
    if (!addr) {
        printf("failed!!!\n");
        return;
    }

    pthread_mutex_lock(&vcmtx);
    memcpy((char *) addr, (void *) &_offset, 8);
    memcpy((char *) addr + 8, &_neighbor, 8);
    memcpy((char *) addr + 16, &_active, 8);
    memcpy((char *) addr + 24, &temp, 4);
    memcpy((char *) addr + 28, &_start_v, 4);
    memcpy((char *) addr + 32, &_end_v, 4);
    memcpy((char *) addr + 36, &weight, 8);
    memcpy((char *) addr + 44, &_hid, 4);
    memcpy((char *) addr + 48, &vertex_data, 8);
    shmdt(addr);
    __asm__ __volatile__("xchg %r14, %r14");
    pthread_mutex_unlock(&vcmtx);
}

//主程序端接口
inline Edge hats_vo_fetch_edge(int hid)
{
    __asm__ __volatile__("xchg %rdx, %rdx");
    Edge edge;
    int shmId = shmget((key_t)5678, 300, 0666|IPC_CREAT); //获取共享内存标志符
    void *address = shmat(shmId, NULL, 0); //获取共享内存地址

    memcpy(&edge, (char *) address + hid * sizeof(edge), sizeof(edge));
    shmdt(address);
    return edge;
}

#endif //ZSIM_VO_H
