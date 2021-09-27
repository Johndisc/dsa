//
// Created by CGCL on 2021/9/23.
//

#include <iostream>
#include <thread>
#include <unistd.h>

int main()
{
    pid_t pid, pid1, pid2;
    pid=fork();
    if (pid>0)
    {
        printf("parent*****%d\n",getpid());
    }
    else
    {
        pid1 = fork();
        if (pid1>0)
            printf("childp******%d\n", getpid());
        else
            printf("childc******%d\n", getpid());
    }

    return 0;
}
