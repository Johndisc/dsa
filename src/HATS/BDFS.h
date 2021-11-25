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

static pthread_mutex_t bcmtx, bfmtx, amtx;

template <typename T>
class BDFS {
private:
    vector<int> *offset;
    vector<int> *neighbor;
    vector<bool> *active_bits;
    vector<T> *vertex_data;
    bool isPush;
    bool is_end;

    int current_vid;
    int last_vid;
    int cur_depth;

    stack<Edge> dfs_stack;
    queue<Edge> FIFO;
    mutex fifo_mutex;

    uint32_t tid;

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

    void bdfs()
    {
        int start_offset, end_offset;
        bool depin;
        Edge edge;
        cur_depth = 0;
        while (!dfs_stack.empty())
        {
            edge = dfs_stack.top();
            accessL2(tid, (uint64_t) & offset->at(0), true);
            start_offset = (*offset)[edge.v];
            accessL2(tid, (uint64_t) & offset->at(edge.v + 1), true);
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
                pthread_mutex_lock(&amtx);
//                accessL2(tid, (uint64_t) & neighbor->at(i), true);
                if (cur_depth < BDFS_MAX_DEPTH && (*active_bits)[(*neighbor)[i]]) {     //只入队，不遍历
                    (*active_bits)[(*neighbor)[i]] = false;
                    dfs_stack.push(Edge(edge.v, (*neighbor)[i]));
                    depin = true;
                }
                else
                {
                    while (this->FIFO.size() > BDFS_MAX_DEPTH)
                        this_thread::yield();               //fifo满时HATS停止
                    lock_guard<mutex> lock(fifo_mutex);
                    FIFO.push(Edge(edge.v, (*neighbor)[i]));
                }
                pthread_mutex_unlock(&amtx);
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
            bdfs();
            vid = scan();
        }
        is_end = true;
        cout << "traverse end" << endl;
    }

    BDFS(uint32_t _tid){ tid = _tid; };
    ~BDFS()= default;

    // zsim端接口
    void configure(vector<int> *_offset, vector<int> *_neighbor, vector<bool> *_active, vector<T> *_vertex_data, bool _isPush,
                   int _start_v, int _end_v)
    {
        offset = _offset;
//        accessL2(tid, (uint64_t) &offset->at(0), true);
        neighbor = _neighbor;
        active_bits = _active;
        vertex_data = _vertex_data;
        isPush = _isPush;
        current_vid = _start_v;
        last_vid = _end_v;
//        cout << offset.size() << " " << neighbor.size() << " " << current_vid << " " << last_vid << endl;
    }

    // zsim端接口
    void fetchEdges(Edge &edge)
    {
        while (this->FIFO.empty() && !is_end)
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
inline void hats_bdfs_configure(vector<int> *_offset, vector<int> *_neighbor, vector<bool> *_active, bool _isPush, int _start_v, int _end_v)
{
    int *pInt = (int *) malloc(10 * sizeof(int));
    pInt[0] = 10;
    pInt[1] = 100;
    printf("(%d %d)\n", pInt[0], pInt[1]);
    int temp = (int) _isPush;
    vector<int> vertex_data(10, 5), *p = &vertex_data;

    int shmId = shmget((key_t)1234, 100, 0666|IPC_CREAT); //获取共享内存标志符
    void *addr = shmat(shmId, NULL, 0); //获取共享内存地址
    if (!addr)
    {
        printf("failed!!!\n");
        return;
    }

    pthread_mutex_lock(&bcmtx);
    memcpy((char *)addr, (void*)&_offset, 8);
    memcpy((char *)addr + 8, &_neighbor, 8);
    memcpy((char *)addr + 16, &_active, 8);
    memcpy((char *)addr + 24, &temp, 4);
    memcpy((char *)addr + 28, &_start_v, 4);
    memcpy((char *)addr + 32, &_end_v, 4);
    memcpy((char *)addr + 36, &p, 8);
    memcpy((char *)addr + 44, &pInt, sizeof(int*));
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

    memcpy(&edge.u,(char *)address+80,4);
    memcpy(&edge.v,(char *)address+84,4);
    shmdt(address);
    pthread_mutex_unlock(&bfmtx);
    return edge;
}

#endif //ZSIM_BDFS_H
