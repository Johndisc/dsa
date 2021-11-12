//
// Created by CGCL on 2021/10/18.
//

#include <iostream>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

void callAsm()
{
    __asm__ __volatile__("xchg %rdx, %rdx");
//    printf("original\n");
}

void memoryMap(vector<int> *node)
{
    long out = 4, in = 12;
    cout << node << endl;
    __asm__ __volatile__("movq %1, %%r15;xchg %%rdx, %%rdx; movq %%r15, %0; ":"=g"(out):"g"(node):"%rdx");
//    printf("%lx\n", out);
    __asm__ __volatile__("movq %1, 0x666666;movq %%rax, %0":"=r"(out):"r"(node));
//    __asm__ __volatile__("movq %1, %%rax;movq %%rax, %%rbx;movq %%rax, %0":"=r"(out):"r"(node):"%rax");
}

int main()
{
    callAsm();
//    vector<int> node;
//    for (int i = 0; i < 10; ++i)
//        node.push_back((i + 1) * 100);
//    callAsm();
//    memoryMap(&node);
//    cout << node[0] << endl;
    return 0;
}