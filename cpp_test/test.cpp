//
// Created by CGCL on 2021/9/30.
//

#include "../src/HATS/VO.h"
#include "../src/HATS/BDFS.h"
#include <cstdlib>
#include <thread>
#include <iostream>
#include <ctime>
#include "omp.h"

#define VEX_NUM 200
#define THREAD_NUM 16
#define TSIZE VEX_NUM / THREAD_NUM
//#define BDFS

#ifdef BDFS
#define hats_config hats_bdfs_configure
#define hats_fetch hats_bdfs_fetch_edge
#else
#define hats_config hats_vo_configure
#define hats_fetch hats_vo_fetch_edge
#endif

using namespace std;

vector<int> offsets, neighbors, values, vertex_data;
bool isPush;
pthread_mutex_t pmutex;

void generate()
{
    srand((int) (time(NULL)));
    int vnum = VEX_NUM, cnt = 0;
    double r;
    offsets.push_back(0);
    for (int i = 0; i < vnum; ++i) {
        for (int j = 0; j < vnum; ++j) {
            if (i == j)
                continue;
            r = (double) rand() / RAND_MAX;
            if (r < 0.1) {
                neighbors.push_back(j);
                values.push_back(i);
                cnt++;
            }
        }
        offsets.push_back(cnt);
    }
}

void newTraverse(vector<bool> *active, bool isPush, int start_id, int end_id, int tid, int &cnt)
{
    hats_config(&offsets, &neighbors, active, isPush, start_id, end_id);
    Edge edge(0, 0);
    cnt = 0;
    while (edge.u != -1 && edge.v != -1) {
//        edge = hats_fetch();
        cnt++;
        if (cnt == 10) {
            edge.u = -1;
            edge.v = -1;
        }
//        pthread_mutex_lock(&pmutex);
//        cout << tid << ":" << cnt << ":" << "(" << edge.u << "," << edge.v << ")" << endl;
//        pthread_mutex_unlock(&pmutex);
    }
    cnt--;
}

void thread_parallel(vector<bool> &active, int *cnt)
{
    thread t[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i)
        t[i] = thread(newTraverse, &active, isPush, i * TSIZE, (i + 1) * TSIZE, i + 1, ref(cnt[i]));
    printf("\nwaiting...\n");
    for (auto & i : t)
        i.join();
}

void omp_parallel(vector<bool> &active, int *cnt)
{
    omp_set_num_threads(THREAD_NUM);
#pragma omp parallel for
    for (int i = 0; i < THREAD_NUM; ++i) {
        newTraverse(&active, isPush, i * TSIZE, (i + 1) * TSIZE, i + 1, cnt[i]);
    }
}

void check_result(int *cnt)
{
    int total = 0;
    for (int i = 0; i < THREAD_NUM; ++i)
        total += cnt[i];
    cout << "total:" << total << endl;
    if (total == neighbors.size())
        cout << "correct" << endl;
    else
        cout << "wrong " << "total:" << total << " size:" << neighbors.size() << endl;
}

int main()
{
    generate();

    cout << offsets.size() << " : ";
//    for (int i:offsets)
//        cout << i << " ";
    cout << endl;
    cout << neighbors.size() << " : ";
//    for (int i:neighbors)
//        cout << i << " ";
    printf("\n---------------\n");
    vector<bool> active(offsets.size() - 1, true);
    for (int i = 0; i < active.size(); ++i)
        vertex_data.push_back(100 * i);
    isPush = true;
    int cnt[THREAD_NUM];

    omp_parallel(active, cnt);
    check_result(cnt);

    return 0;
}