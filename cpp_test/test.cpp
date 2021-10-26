//
// Created by CGCL on 2021/9/30.
//

#include "../src/HATS/VA.h"
#include <cstdlib>
#include <thread>
#include <iostream>
#include <ctime>

using namespace std;

vector<int> offsets, neighbors, values;
mutex sm;

void generate()
{
    srand((int) (time(NULL)));
    int vnum = 20, cnt = 0;
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

void fetch(VA<int> &VAs, int start, int end, int &num)
{
    Edge edge(1, 1);
    while (edge.v != -1) {
        VAs.hats_fetch_edges(edge);
        printf("(%d,%d):%d, %d\n", start, end, edge.u, edge.v);
        num++;
    }
    num--;
}

void traverse(vector<bool> *active, vector<int> vertex_data, bool isPush, int start_id, int end_id)
{
    VA<int> VAs;
    int num = 0;
    thread wthread(&VA<int>::hats_configure, &VAs, offsets, neighbors, active, vertex_data, isPush, start_id, end_id);
    thread rthread(fetch, ref(VAs), start_id, end_id, ref(num));
    wthread.join();
    rthread.join();
    printf("wt(%d,%d) end, num=%d\n", start_id, end_id, num);
}

void newTraverse(vector<bool> *active, bool isPush, int start_id, int end_id, int tid)
{
    configure(&offsets, &neighbors, active, isPush, start_id, end_id);
    Edge edge(0, 0);
    int cnt = 0;
    while (edge.u != -1 && edge.v != -1) {
        edge = fetchEdge();
        cnt++;
        cout << tid << ":" << cnt << ":" << "(" << edge.u << "," << edge.v << ")" << endl;
    }
}

int main()
{
//    vector<int> offsets = {0, 2, 4, 7, 9};
//    vector<int> neighbors = {0, 1, 1, 2, 0, 2, 3, 1, 3};
//    vector<int> values = {1, 7, 2, 8, 5, 3, 9, 6, 4};

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
//    configure(&offsets, &neighbors, &active, isPush, 0, 20);
//    Edge edge(0, 0);
//    int cnt = 0;
//    while (edge.u != -1 && edge.v != -1) {
//        edge = fetchEdge();
//        cnt++;
//        cout << cnt << ":" << "(" << edge.u << "," << edge.v << ")" << endl;
//    }
    thread t[4];
    for (int i = 0; i < 4; ++i) {
        t[i] = thread(newTraverse, &active, isPush, i * 5, (i + 1) * 5, i + 1);
    }
    printf("\nwaiting...\n");
    for (auto & i : t) {
        i.join();
    }
    return 0;
}