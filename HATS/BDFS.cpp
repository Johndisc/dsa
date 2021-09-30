//
// Created by CGCL on 2021/9/17.
//

#include "BDFS.h"

using namespace std;

template<typename T>
BDFS<T>::BDFS(vector<int> _offset, vector<int> _neighbor, vector <T> _vertex_data, bool _isPush, int _start_v,
              int _end_v):offset(_offset), neighbor(_neighbor), vertex_data(_vertex_data), isPush(_isPush),
                          start_v(_start_v) {}

template <typename T>
BDFS<T>::~BDFS<T>() = default;

template <typename T>
void BDFS<T>::hats_configure(v) {

}

template <typename T>
void BDFS<T>::hats_fetch_edges() {
    
}