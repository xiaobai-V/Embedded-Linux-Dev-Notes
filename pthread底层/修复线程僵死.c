#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
// 线程函数：修复线程僵死
void *file_read_safe_routine(void *arg)
{
    // 以非阻塞方式打开文件
    int fd = open("/dev/exception_device", O_RDONLY | O_NONBLOCK);
    if (fd == -1)
    {
        perror("文件打开失败");
        return NULL;
    }
    // 设置 1 秒超时
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    // 等待数据可读，超时则退出
    int ready = select(fd + 1, &read_fds, NULL, NULL, &tv);
    if (ready == -1)
    {
        perror("select 错误");
        close(fd);
        return NULL;
    }
    else if (ready == 0)
    {
        printf("读取超时，线程安全退出，避免僵死\n");

        close(fd);
        return NULL;
    }
    // 正常读取
    char buffer[1024];
    read(fd, buffer, sizeof(buffer));

    printf("文件读取成功\n");
    close(fd);
    return NULL;
}

int main(int argc, char const *argv[])
{
    pthread_t tid;
    pthread_create(&tid, NULL, file_read_safe_routine, NULL);
    // 主线程等待僵死线程，永远无法返回
    pthread_join(tid, NULL);

    return 0;
}
