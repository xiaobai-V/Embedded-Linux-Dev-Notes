#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * 有名管道（FIFO）—— 读端
 *
 * 运行方式：先在另一个终端启动 ./fifo_write（创建 FIFO 并阻塞等待 reader），
 *          再启动本程序，两端同时解除阻塞，开始通信
 */

int main(int argc, char const *argv[])
{
    char *fifoPath = "/tmp/myfifo";

    // 【存在性检查】如果先启动 reader，FIFO 文件还未被 writer 创建
    // 直接 open 会失败（ENOENT），给出明确提示而非模糊的 perror
    if (access(fifoPath, F_OK) == -1)
    {
        fprintf(stderr, "FIFO 文件 %s 不存在，请先启动 fifo_write 创建管道\n", fifoPath);
        exit(EXIT_FAILURE);
    }

    // 【阻塞语义】open(O_RDONLY) 在没有 writer 打开写端之前会一直阻塞
    // 必须等 writer 也调用了 open(O_WRONLY)，两端才同时通过
    int fd;
    fd = open(fifoPath, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 从有名管道中读取数据，打印到控制台
    ssize_t readNum;
    char buf[100];
    // read 返回 0 表示写端全部关闭（EOF）aa，此时退出循环
    // 这与匿名管道的行为一致：所有写端 fd 关闭后，read 不再阻塞
    while ((readNum = read(fd, buf, sizeof(buf))) > 0)
    {
        write(STDOUT_FILENO, buf, readNum);
    }

    if (readNum == -1)
    {
        perror("read fifo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("\n接收管道退出，进程终止\n");
    close(fd);

    return 0;
}
