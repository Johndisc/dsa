//
// Created by CGCL on 2021/10/22.
//

#ifndef ZSIM_SHARED_MEMORY_H
#define ZSIM_SHARED_MEMORY_H

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>

using namespace std;
const char *path = "tempIpc";

int createSharedMem()
{
    int shmId;
    key_t key = ftok(path, 1);
    if (key == -1) {
        cout << "ftok failed" << endl;
        return -1;
    }
    shmId = shmget(key, 100, IPC_CREAT|IPC_EXCL | 0600); //获取共享内存标志符
    if (shmId == -1) {
        cout << "shmget failed" << endl;
        return -1;
    }
    return shmId;
}

void* getSharedMemAddr()
{
    key_t key = ftok(path, 1);
    if (key == -1) {
        cout << "ftok failed" << endl;
        return nullptr;
    }
    int shmId = shmget(key, 0, 0); //获取共享内存标志符
    void *shmaddr = shmat(shmId, NULL, 0); //获取共享内存地址
    return shmaddr;
}

#endif //ZSIM_SHARED_MEMORY_H
