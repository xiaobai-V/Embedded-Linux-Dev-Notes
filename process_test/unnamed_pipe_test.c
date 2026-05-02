#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// 问题1：为什么不用担心子进程先执行，close(pipefd[1])关闭写端，导致父进程无法写入管道？
// fork 后，管道读写端各有两份拷贝
// 子进程关闭写端 = 只关自己那份
// 只要父进程写端不关，管道永远可写

// 问题2：

int main(int argc, char const *argv[])
{
    // 功能：将程序传进来的第一个参数 通过管道 传输给子进程
    int pipefd[2];
    pid_t cpid;
    char buf;
    if (argc != 2)
    {
        fprintf(stderr, "%s:请填写需要传递的信息\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /**
     * @brief 简述：创建匿名管道
     *
     * @def 函数原型：int pipe (int __pipedes[2])
     *
     * @param int __pipedes[2]：
     *      含义：用于返回指向管道两端的两个文件描述符
     *      __pipedes[0]：读端
     *      __pipedes[1]：写端
     *
     * @return 返回值：
     *      成功：0
     *      失败：-1
     *
     * @note 注意事项/易错点：
     */
    if (pipe(pipefd) == -1)
    {
        perror("创建管道失败");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (cpid == 0)
    {
        // 子进程 读数据  打印到控制台
        // 关闭写端
        // 这里父子进程的pidefd只想到
        close(pipefd[1]);
        printf("子进程%d读出数据\n", getpid());
        while (read(pipefd[0], &buf, sizeof(buf)) > 0)
        {
            write(STDOUT_FILENO, &buf, sizeof(buf));
        }
        write(STDOUT_FILENO, "\n", 1);
        close(pipefd[0]);
        _exit(EXIT_SUCCESS); // 子进程退出，不进行清理
    }
    else
    {
        // 父进程 写数据到管道
        // 关闭读端
        close(pipefd[0]);
        printf("父进程%d写入数据\n", getpid());
        write(pipefd[1], argv[1], strlen(argv[1]));
        close(pipefd[1]);
        waitpid(cpid, NULL, 0); // 等待子进程结束，防止出现孤儿进程
        exit(EXIT_SUCCESS);     // 父进程退出，进行清理
    }

    return 0;
}
