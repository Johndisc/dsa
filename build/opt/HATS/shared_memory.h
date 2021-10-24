//
// Created by CGCL on 2021/10/22.
//

#ifndef ZSIM_SHARED_MEMORY_H
#define ZSIM_SHARED_MEMORY_H

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>
#include <string>

using namespace std;
//static string path = "/home/jzh/zsim_en/zsim/src/HATS/tempIpc";

static int createSharedMem()
{
    int shmId;
//    key_t key = ftok(path.c_str(), 1);
//    if (key == -1) {
//        cout << "ftok failed" << endl;
//        return -1;
//    }
    shmId = shmget((key_t) 1234, 100, 0666 | IPC_CREAT); //获取共享内存标志符
    if (shmId == -1) {
        cout << "shmget failed" << endl;
        return -1;
    }
    return shmId;
}

static void* getSharedMemAddr()
{
//    const char *cpath = "asd";
//    key_t key = ftok(path.c_str(), 1);
//    if (key == -1) {
//        cout << "ftok failed" << endl;
//        return nullptr;
//    }
    int shmId = shmget((key_t)1234, 0, 0); //获取共享内存标志符
    void *shmaddr = shmat(shmId, NULL, 0); //获取共享内存地址
    return shmaddr;
}

#endif //ZSIM_SHARED_MEMORY_H
