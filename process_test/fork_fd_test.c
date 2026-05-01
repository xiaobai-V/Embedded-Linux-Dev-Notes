#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// 几个问题
// 1. 文件是在父进程中打开的，子进程中为什么也能进行写入
// 子进程能写：fork 会复制父进程的文件描述符，父子共享同一个文件表项，所以子进程能直接读写已打开的文件。
// 2. 子进程和父进程都往文件中写数据，会不会造成冲突
// 不会冲突：文件用了 O_APPEND 模式，内核保证写入原子性，数据会按顺序追加，不会覆盖、不混乱。

int main(int argc, char const *argv[])
{
    // fork之前
    // 打开一个文件

    int fd = open("io.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buffer[1024]; // 缓冲区存放写出的数据
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // 子进程代码
        strcpy(buffer, "这是子进程写入的数据！\n");
    }
    else
    {
        // 父进程代码
        sleep(1);
        strcpy(buffer, "这是子进程写入的数据！\n");
    }

    // 父子进程都执行的代码
    ssize_t bytes_write = write(fd, buffer, strlen(buffer));
    if (bytes_write == -1)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // 使用完毕之后关闭
    close(fd);

    if (pid == 0)
    {
        printf("子进程写入完毕，并释放文件描述符\n");
    }
    else
    {
        printf("父进程写入完毕，并释放文件描述符\n");
    }
    return 0;
}
