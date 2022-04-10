//
// Created by CGCL on 2021/9/30.
//

#include "../src/HATS/VO.h"
#include "../src/HATS/BDFS.h"
#include <cstdlib>
#include <thread>
#include <iostream>
#include <fstream>
#include <ctime>
#include "omp.h"

#define VEX_NUM 128
#define THREAD_NUM 16
#define TSIZE VEX_NUM / THREAD_NUM
#define BDFS

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

void readFromFile()
{
    ifstream ifs;
    ifs.open("/home/jzh/jzh/ligra/inputs/rMatGraph_J_5_100", ios::in);

    string s;
    int mid, o_size, n_size;
    getline(ifs, s);
    getline(ifs, s);
    o_size = atoi(s.c_str());
    getline(ifs, s);
    n_size = atoi(s.c_str());
    offsets.resize(o_size);
    neighbors.resize(n_size);
    values.resize(n_size);
    for (int i = 0; i < o_size; ++i) {
        getline(ifs, s);
        mid = atoi(s.c_str());
        offsets[i] = mid;
    }
    offsets.push_back(n_size);
    for (int i = 0; i < n_size; ++i) {
        getline(ifs, s);
        mid = atoi(s.c_str());
        neighbors[i] = values[i] = mid;
    }
    ifs.close();
}

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

void coreTraverse(vector<bool> *active,int start_id, int end_id, int tid, int &cnt)
{
    cnt = 0;
    int sn, en;
    Edge edge;
    for (int i = start_id; i < end_id; ++i) {
        if (active->at(i)) {
            sn = offsets[i];
            en = offsets[i + 1];
            for (int j = sn; j < en; ++j) {
                edge = Edge(i, neighbors[j]);
                cnt++;
            }
        }
    }
}

void hatsTraverse(vector<bool> *active, int start_id, int end_id, int hid, int &cnt)
{
    hats_config(&offsets, &neighbors, &vertex_data, active, isPush, start_id, end_id, hid);
    Edge edge(0, 0);
    cnt = 0;
    while (edge.u != -1 && edge.v != -1) {
        edge = hats_fetch(hid);
        cnt++;
    }
    cnt--;
}

void thread_parallel(vector<bool> &active, int *cnt)
{
    thread t[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i)
        t[i] = thread(hatsTraverse, &active, i * TSIZE, (i + 1) * TSIZE, i, ref(cnt[i]));
    printf("\nwaiting...\n");
    for (auto & i : t)
        i.join();
}

void omp_parallel(vector<bool> &active, int *cnt)
{
    omp_set_num_threads(THREAD_NUM);
#pragma omp parallel for
    for (int i = 0; i < THREAD_NUM; ++i) {
        hatsTraverse(&active, i * TSIZE, (i + 1) * TSIZE, i, cnt[i]);
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
//    generate();
    readFromFile();

    cout << offsets.size() << " " << neighbors.size() << endl;
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