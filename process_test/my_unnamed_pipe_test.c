#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    // 场景：父进程创建管道，fork两个子进程，子1写→子2读

    int pipefd[2]; // pipefd[0]=读端, pipefd[1]=写端
    pid_t cpid1, cpid2;
    char buf[256];

    if (pipe(pipefd) == -1)
    {
        perror("创建管道失败");
        return -1;
    }

    // ===== 子进程1（写端） =====
    cpid1 = fork();
    if (cpid1 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid1 == 0)
    {
        // 子进程1：只写，关闭读端
        close(pipefd[0]);
        printf("子进程1-%d向管道写数据\n", getpid());
        sprintf(buf, "弟弟你好，我是%d\n", getpid());
        write(pipefd[1], buf, strlen(buf));
        close(pipefd[1]); // 写完关闭，子2才能收到EOF
        _exit(EXIT_SUCCESS);
    }

    // 【关键】父进程立即关闭自己的写端
    // 原因：fork后写端引用计数=2（父+子1），子1关闭后还有父进程的1份
    //       若父进程不关，子2的read()永远不会返回EOF（以为还有人会写）
    //       父进程提前关闭→子1关闭时引用计数归零→内核向子2发送EOF
    close(pipefd[1]);

    // ===== 子进程2（读端） =====
    cpid2 = fork();
    if (cpid2 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid2 == 0)
    {
        // 子进程2：只读，关闭写端（自己从父进程继承来的那份）
        close(pipefd[1]);
        printf("子进程2-%d从管道读数据\n", getpid());
        char ch;
        while (read(pipefd[0], &ch, 1) > 0)
        {
            write(STDOUT_FILENO, &ch, 1);
        }
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);
    }

    // 父进程关闭读端（不再需要），等待两个子进程退出
    close(pipefd[0]);
    waitpid(cpid1, NULL, 0);
    waitpid(cpid2, NULL, 0);

    return 0;
}
