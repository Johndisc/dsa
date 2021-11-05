//
// Created by CGCL on 2021/9/30.
//

#include "../src/HATS/VO.h"
#include "../src/HATS/BDFS.h"
#include <cstdlib>
#include <thread>
#include <iostream>
#include <ctime>

#define VEX_NUM 20

using namespace std;

vector<int> offsets, neighbors, values;
mutex sm;
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
//                printf("%d,%d\n", i, j);
            }
        }
        offsets.push_back(cnt);
    }
}

void fetch(VO<int> &VAs, int start, int end, int &num)
{
    Edge edge(1, 1);
    while (edge.v != -1) {
        VAs.fetchEdges(edge);
        printf("(%d,%d):%d, %d\n", start, end, edge.u, edge.v);
        num++;
    }
    num--;
}

void traverse(vector<bool> *active, vector<int> vertex_data, bool isPush, int start_id, int end_id)
{
    VO<int> VAs;
    int num = 0;
    thread wthread(&VO<int>::configure, &VAs, offsets, neighbors, active, vertex_data, isPush, start_id, end_id);
    thread rthread(fetch, ref(VAs), start_id, end_id, ref(num));
    wthread.join();
    rthread.join();
    printf("wt(%d,%d) end, num=%d\n", start_id, end_id, num);
}

void newTraverse(vector<bool> *active, bool isPush, int start_id, int end_id, int tid, int &cnt)
{
    hats_bdfs_configure(&offsets, &neighbors, active, isPush, start_id, end_id);
    Edge edge(0, 0);
    cnt = 0;
    while (edge.u != -1 && edge.v != -1) {
        edge = hats_bdfs_fetch_edge();
        cnt++;
        pthread_mutex_lock(&pmutex);
        cout << tid << ":" << cnt << ":" << "(" << edge.u << "," << edge.v << ")" << endl;
        pthread_mutex_unlock(&pmutex);
    }
    cnt--;
}

int main()
{
    generate();

    cout << offsets.size() << " : ";
    for (int i:offsets)
        cout << i << " ";
    cout << endl;
    cout << neighbors.size() << " : ";
    for (int i:neighbors)
        cout << i << " ";
    printf("\n---------------\n");
    vector<bool> active(offsets.size() - 1, true);
    vector<int> vertex_data;
    for (int i = 0; i < active.size(); ++i)
        vertex_data.push_back(100 * i);
    bool isPush = true;

    int tnum = 1, tsize = VEX_NUM / tnum;
    thread t[tnum];
    int cnt[tnum], total = 0;
    for (int i = 0; i < tnum; ++i)
        t[i] = thread(newTraverse, &active, isPush, i * tsize, (i + 1) * tsize, i + 1, ref(cnt[i]));
    printf("\nwaiting...\n");
    for (auto & i : t)
        i.join();
    for (int i = 0; i < tnum; ++i)
        total += cnt[i];
    cout << "total:" << total << endl;
    if (total==neighbors.size())
        cout << "correct" << endl;
    else
        cout << "wrong " << "total:" << total << " size:" << neighbors.size() << endl;
    return 0;
}