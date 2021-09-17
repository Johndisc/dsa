//
// Created by CGCL on 2021/9/17.
//

#include "BDFS.h"

using namespace std;

template <typename T>
BDFS<T>::BDFS() {}

template <typename T>
BDFS<T>::~BDFS<T>() = default;

template <typename T>
void BDFS<T>::hats_configure(vector<int> offset, vector<int> neighbor, vector <T> vertex_data, bool isPush, int start_v,
                             int end_v) {
    offset = offset;
    neighbor = neighbor;
    vertex_data = vertex_data;
    isPush = isPush;
    start_v = start_v;
    end_v = end_v;
}

template <typename T>
void BDFS<T>::hats_fetch_edges() {
    
}