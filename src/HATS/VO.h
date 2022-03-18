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

static pthread_mutex_t vcmtx, vfmtx, vL2mtx;

template <typename T>
class VO {
private:
    vector<int> *offset;
    vector<int> *neighbor;
    vector<bool> *active_bits;
    vector<T> *vertex_data;
    bool isPush;

    int current_vid;
    int last_vid;

    queue<Edge> FIFO;
    mutex fifo_mutex;
    uint32_t tid;

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
        pthread_mutex_lock(&vL2mtx);
        start_offset = (*offset)[vid];
        accessL2(tid, (uint64_t) & offset->at(vid), true);
        end_offset = (*offset)[vid + 1];
        accessL2(tid, (uint64_t) &offset->at(vid + 1), true);
        pthread_mutex_unlock(&vL2mtx);
    }

    vector<Edge> fetch_neighbors(int vid, int start_offset, int end_offset)
    {
        vector <Edge> neighbors;
        for (int i = start_offset; i < end_offset; ++i) {
            while (this->FIFO.size() > VO_MAX_DEPTH)
                this_thread::yield();               //fifo满时HATS停止
            lock_guard<mutex> lock(fifo_mutex);
            FIFO.push(Edge(vid, (*neighbor)[i]));
            pthread_mutex_lock(&vL2mtx);
            prefetch((*neighbor)[i]);
            accessL2(tid, (uint64_t) & neighbor->at(i), true);
            pthread_mutex_unlock(&vL2mtx);
            neighbors.emplace_back(vid, i);
        }
        return neighbors;
    }

    void prefetch(int vid)
    {
        accessL2(tid, (uint64_t) & vertex_data->at(vid), true);
    }

public:
    void start()
    {
        int vid, start_offset, end_offset;
        vector <Edge> edges;
        //-------------------
//        for (int i = 0; i < 5; ++i) {
//            printf("%d  in   %d times\n", tid, i);
//        }
        //-------------------
        while (current_vid < last_vid) {
            vid = scan();
            fetch_offsets(vid, start_offset, end_offset);
            edges = fetch_neighbors(vid, start_offset, end_offset);
        }
        current_vid++;
        cout << "traverse end" << endl;
    }

    VO(uint32_t _tid){ tid = _tid; };
    ~VO()= default;

    //zsim端接口
    void configure(vector<int> *_offset, vector<int> *_neighbor, vector<bool> *_active, vector<T> *_vertex_data, bool _isPush,
                   int _start_v, int _end_v)
    {
        offset = _offset;
        neighbor = _neighbor;
        active_bits = _active;
        vertex_data = _vertex_data;
        isPush = _isPush;
        current_vid = _start_v;
        last_vid = _end_v;
    }

    //zsim端接口
    void fetchEdges(Edge &edge)
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

//主程序端接口
inline void hats_vo_configure(vector<int> *_offset, vector<int> *_neighbor, vector<bool> *_active, bool _isPush, int _start_v, int _end_v)
{
    int temp = (int) _isPush;
    vector<int> vertex_data(_offset->size() - 1, 5), *p = &vertex_data;

    int shmId = shmget((key_t)1234, 100, 0666|IPC_CREAT); //获取共享内存标志符
    void *addr = shmat(shmId, NULL, 0); //共享内存地址
    if (!addr)
    {
        printf("failed!!!\n");
        return;
    }

    pthread_mutex_lock(&vcmtx);
    memcpy((char *)addr, (void*)&_offset, 8);
    memcpy((char *)addr + 8, &_neighbor, 8);
    memcpy((char *)addr + 16, &_active, 8);
    memcpy((char *)addr + 24, &temp, 4);
    memcpy((char *)addr + 28, &_start_v, 4);
    memcpy((char *)addr + 32, &_end_v, 4);
    memcpy((char *)addr + 36, &p, 8);
    shmdt(addr);
    __asm__ __volatile__("xchg %r15, %r15");
    pthread_mutex_unlock(&vcmtx);
}

//主程序端接口
inline Edge hats_vo_fetch_edge()
{
    pthread_mutex_lock(&vfmtx);
    __asm__ __volatile__("xchg %rdx, %rdx");
    Edge edge;
    int shmId = shmget((key_t)1234, 100, 0666|IPC_CREAT); //获取共享内存标志符
    void *address = shmat(shmId, NULL, 0); //获取共享内存地址

    memcpy(&edge.u,(char *)address+80,4);
    memcpy(&edge.v,(char *)address+84,4);
    shmdt(address);
    pthread_mutex_unlock(&vfmtx);
    return edge;
}

#endif //ZSIM_VO_H
