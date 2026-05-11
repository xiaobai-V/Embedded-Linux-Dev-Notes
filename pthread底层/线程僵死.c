#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
// 线程函数：模拟出现I/O异常导致线程僵死
void *file_read_routine(void *arg)
{
    // 打开一个不存在或异常的设备/文件
    int fd = open("/dev/exception_device", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return NULL;
    }

    char buf[1024];
    // 无超时、处理异常，异常时会永久阻塞，导致线程僵死
    ssize_t nread = read(fd, buf, sizeof(buf));
    if (nread < 0)
    {
        perror("read");
        return NULL;
    }
    // 若I/O异常卡死，这里不会执行
    printf("Read %zd bytes: %s", nread, buf);
    close(fd);
    return NULL;
}

int main(int argc, char const *argv[])
{
    pthread_t tid;
    pthread_create(&tid, NULL, file_read_routine, NULL);
    // 主线程等待僵死线程，永远无法返回
    pthread_join(tid, NULL);

    return 0;
}
