//
// Created by CGCL on 2021/9/28.
//

#ifndef ZSIM_BDFS_H
#define ZSIM_BDFS_H

#include <cstring>
#include <vector>
#include <map>
#include <stack>
#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
#include "Edge.h"
#include "../zsim.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#define BDFS_MAX_DEPTH 10

using namespace std;

void accessL2(uint32_t tid, uint64_t address, bool isLoad);

static pthread_mutex_t bcmtx, bfmtx;

class BDFS:public HATS {
private:
    vector<int> *offset;
    vector<int> *neighbor;
    vector<bool> *active_bits;
    vector<int> *vertex_data;
    bool isPush;
    bool is_end;

    int current_vid;
    int last_vid;
    int cur_depth;

    stack<Edge> dfs_stack;
    queue<Edge> FIFO;
    mutex fifo_mutex;

    uint32_t tid;
    int hats_stall;
    int core_stall;

private:
    // 将要以某个节点为u时，将其设置为unactive
    int scan()
    {
        for (int i = current_vid; i < last_vid; i++) {
            if ((*active_bits)[i]) {
                (*active_bits)[i] = false;
                dfs_stack.push(Edge(-1, i));
                return i;
            }
        }
        return -1;
    }

    void rebdfs(int cid, int depth) {
        int start_offset = (*offset)[cid];
        int end_offset = (*offset)[cid + 1];
        accessL2(tid, (uint64_t) &offset->at(cid), true);
        accessL2(tid, (uint64_t) &offset->at(cid + 1), true);
        for (int i = start_offset; i < end_offset; ++i) {
            while (this->FIFO.size() > BDFS_MAX_DEPTH) {
                hats_stall++;
                this_thread::yield();               //fifo满时HATS停止
            }
            fifo_mutex.lock();
            FIFO.push(Edge(cid, (*neighbor)[i]));
            fifo_mutex.unlock();
            prefetch(neighbor->at(i));
            accessL2(tid, (uint64_t) &neighbor->at(i), true);
            if ((*active_bits)[(*neighbor)[i]] && depth < BDFS_MAX_DEPTH) {
                (*active_bits)[(*neighbor)[i]] = false;
                rebdfs((*neighbor)[i], depth + 1);
            }
        }
    }

    void bdfs()
    {
        int start_offset, end_offset;
        bool depin;
        Edge edge;
        cur_depth = 0;
        while (!dfs_stack.empty())
        {
            edge = dfs_stack.top();
            start_offset = (*offset)[edge.v];
            accessL2(tid, (uint64_t) & offset->at(edge.v + 1), true);
            accessL2(tid, (uint64_t) &offset->at(edge.v), true);
            end_offset = (*offset)[edge.v + 1];
            dfs_stack.pop();
            depin = false;
            if (edge.u != -1)
            {
                while (this->FIFO.size() > BDFS_MAX_DEPTH)
                    this_thread::yield();               //fifo满时HATS停止
                lock_guard<mutex> lock(fifo_mutex);
                FIFO.push(edge);
            }
            for (int i = start_offset; i < end_offset; ++i) {
                accessL2(tid, (uint64_t) & neighbor->at(i), true);
                if (cur_depth < BDFS_MAX_DEPTH && (*active_bits)[(*neighbor)[i]]) {     //只入队，不遍历
                    (*active_bits)[(*neighbor)[i]] = false;
                    dfs_stack.push(Edge(edge.v, (*neighbor)[i]));
                    depin = true;
                }
                else
                {
                    while (this->FIFO.size() > BDFS_MAX_DEPTH) {
                        hats_stall++;
                        this_thread::yield();               //fifo满时HATS停止
                    }
                    lock_guard<mutex> lock(fifo_mutex);
                    FIFO.push(Edge(edge.v, (*neighbor)[i]));
                }
            }
            if (depin)
                cur_depth++;
            else
                cur_depth--;
        }
    }

    void prefetch(int vid)
    {
        accessL2(tid, (uint64_t) & vertex_data->at(vid), true);
    }

public:
    void start()
    {
        is_end = false;
        int vid = scan();
        while (vid != -1) {
            rebdfs(vid, 0);
//            bdfs();
            vid = scan();
        }
        is_end = true;
        printf("thread %d end, core stall:%d hats stall:%d ratio:%f\n", tid, core_stall, hats_stall,
               (double) core_stall / hats_stall);
    }

    BDFS(uint32_t _tid){ tid = _tid; };
    ~BDFS()= default;

    // zsim端接口
    void configure(vector<int> *_offset, vector<int> *_neighbor, vector<bool> *_active, vector<int> *_vertex_data, bool _isPush,
                   int _start_v, int _end_v)
    {
        hats_stall = core_stall = 0;
        offset = _offset;
        neighbor = _neighbor;
        active_bits = _active;
        vertex_data = _vertex_data;
        isPush = _isPush;
        current_vid = _start_v;
        last_vid = _end_v;
    }

    // zsim端接口
    void fetchEdges(Edge &edge)
    {
        while (this->FIFO.empty() && !is_end) {
            printf("core %d stall\n", tid);
            core_stall++;
            this_thread::yield();           //fifo为空时fetch停止
        }
        lock_guard<mutex> lock(fifo_mutex);
        if (!this->FIFO.empty())
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
hats_bdfs_configure(vector<int> *_offset, vector<int> *_neighbor, vector<int> *vertex_data, vector<bool> *_active,
                    bool _isPush, int _start_v, int _end_v) {

    int temp = (int) _isPush;

    int shmId = shmget((key_t) 1234, 100, 0666 | IPC_CREAT); //获取共享内存标志符
    void *addr = shmat(shmId, NULL, 0); //获取共享内存地址
    if (!addr) {
        printf("failed!!!\n");
        return;
    }

    pthread_mutex_lock(&bcmtx);
    memcpy((char *) addr, (void *) &_offset, 8);
    memcpy((char *) addr + 8, &_neighbor, 8);
    memcpy((char *) addr + 16, &_active, 8);
    memcpy((char *) addr + 24, &temp, 4);
    memcpy((char *) addr + 28, &_start_v, 4);
    memcpy((char *) addr + 32, &_end_v, 4);
    memcpy((char *) addr + 36, &vertex_data, 8);
    shmdt(addr);
    __asm__ __volatile__("xchg %r15, %r15");
    pthread_mutex_unlock(&bcmtx);
}

//主程序端接口
inline Edge hats_bdfs_fetch_edge()
{
    pthread_mutex_lock(&bfmtx);
    __asm__ __volatile__("xchg %rdx, %rdx");
    Edge edge;
    int shmId = shmget((key_t)1234, 100, 0666|IPC_CREAT); //获取共享内存标志符
    void *address = shmat(shmId, NULL, 0); //获取共享内存地址

    memcpy(&edge,(char *)address+80, sizeof(edge));
    shmdt(address);
    pthread_mutex_unlock(&bfmtx);
    return edge;
}

#endif //ZSIM_BDFS_H
