//
// Created by CGCL on 2021/9/30.
//

#include "VA.h"
#include <cstdlib>
#include <iostream>

using namespace std;

void generate(vector<int> &offsets, vector<int> &neighbors, vector<int> &values)
{
    int vnum = 10, cnt = 0;
    double r;
    offsets.push_back(0);
    for (int i = 0; i < vnum; ++i) {
        for (int j = 0; j < vnum; ++j) {
            if (i == j)
                continue;
            r = rand() / RAND_MAX;
            if (r < 0.3) {
                neighbors.push_back(j);
                cnt++;
            }
        }
        offsets.push_back(cnt);
    }
}

int main()
{
//    vector<int> offsets = {0, 2, 4, 7, 9};
//    vector<int> neighbors = {0, 1, 1, 2, 0, 2, 3, 1, 3};
//    vector<int> values = {1, 7, 2, 8, 5, 3, 9, 6, 4};
    vector<int> offsets, neighbors, values;
    generate(offsets, neighbors, values);
    for (int i:offsets)
        cout << i << " ";
    cout << endl;
    for (int i:neighbors)
        cout << i << " ";
}