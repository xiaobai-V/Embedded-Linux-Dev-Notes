#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * 有名管道（FIFO）—— 写端
 *
 * 与匿名管道的区别：
 *   匿名管道：只能用于有亲缘关系的进程（父子进程）
 *   有名管道：任意两个独立进程都能通过文件系统路径找到对方
 *
 * 运行方式：先启动本程序（会阻塞等待 reader），再在另一个终端启动 ./fifo_read
 */

int main(int argc, char const *argv[])
{
    char *fifoPath = "/tmp/myfifo";

    // 【幂等创建】如果 FIFO 已存在（errno == EEXIST），跳过报错继续执行
    // 这样重复运行不会因文件已存在而失败
    if (mkfifo(fifoPath, 0664) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo failed");
            exit(EXIT_FAILURE);
        }
    }

    // 【阻塞语义】open(O_WRONLY) 在没有 reader 打开读端之前会一直阻塞
    // 这与普通文件的 open 完全不同——FIFO 必须读写两端同时就绪才能通过
    // 一旦 reader 也调用了 open(O_RDONLY)，两端同时解除阻塞
    int fd;
    fd = open(fifoPath, O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 从控制台读取用户输入，写入有名管道
    ssize_t readNum;
    char buf[100];
    while ((readNum = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
    {
        write(fd, buf, readNum);
    }

    if (readNum == -1)
    {
        perror("read stdin");
        close(fd);
        exit(1);
    }

    printf("发送管道退出，进程终止\n");
    close(fd); // 关闭写端 → reader 的 read() 返回 0（EOF）

    // 【unlink 语义】删除文件系统中的目录项（文件名）
    // 但如果 reader 仍持有打开的 fd，inode 不会立即释放，reader 可以读完剩余数据
    // 注意：即使 FIFO 不是本进程创建的（errno == EEXIST），这里也会删除
    if (unlink(fifoPath) == -1)
    {
        perror("fifoPath file unlink");
    }
    return 0;
}
