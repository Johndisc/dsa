#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <omp.h>
using namespace std;

//线程要做的事情就写在这个线程函数中
void GetSumT(vector<int>::iterator first,vector<int>::iterator last,int &result)
{
    for (auto it = first; it < last; it++) {
        result += *it;
    }
}

int main() //主线程
{
    int result1,result2,result3,result4,result5;
    result1 = result2 = result3 = result4 = result5 = 0;
    vector<int> largeArrays;
    for(int i=0;i<10000;i++)
    {
        if(i%2!=0)
            largeArrays.push_back(i);
        else
            largeArrays.push_back(-1*i);
    }
    thread first(GetSumT,largeArrays.begin(),
                 largeArrays.begin()+2000,std::ref(result1)); //子线程1
    thread second(GetSumT,largeArrays.begin()+2000,
                  largeArrays.begin()+4000,std::ref(result2)); //子线程2
    thread third(GetSumT,largeArrays.begin()+4000,
                 largeArrays.begin()+6000,std::ref(result3)); //子线程3
    thread fouth(GetSumT,largeArrays.begin()+6000,
                 largeArrays.begin()+8000,std::ref(result4)); //子线程4
    thread fifth(GetSumT,largeArrays.begin()+8000,
                 largeArrays.end(),std::ref(result5)); //子线程5

    first.join(); //主线程要等待子线程执行完毕
    second.join();
    third.join();
    fouth.join();
    fifth.join();
    cout << result1 << " " << result2 << " " << result3 << " " << result4 << " " << result5 << endl;
    int resultSum = result1+result2+result3+result4+result5; //汇总各个子线程的结果
    cout << resultSum << "***********" << endl;
    return 0;
}