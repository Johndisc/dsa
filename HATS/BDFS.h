//
// Created by CGCL on 2021/9/17.
//

#ifndef ZSIM_BDFS_H
#define ZSIM_BDFS_H

#include <vector>

using namespace std;

template <typename T>
class BDFS {
private:
    vector<int> offset;
    vector<int> neighbor;
    vector<bool> active;
    vector<T> vertex_data;
    bool isPush;
    int start_v, end_v;
public:
    BDFS(vector<int> _offset, vector<int> _neighbor, vector <T> _vertex_data, bool _isPush, int _start_v,
         int _end_v);
    ~BDFS();
    void hats_configure(vector<int> offset, vector<int> neighbor, vector<T> vertex_data, bool isPush, int start_v, int end_v);
    void hats_fetch_edges();
};


#endif //ZSIM_BDFS_H
