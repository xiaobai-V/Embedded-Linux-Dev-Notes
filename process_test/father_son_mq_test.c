#define _POSIX_C_SOURCE 200112L

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *mq_name = "/father_son_mq";
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    attr.mq_curmsgs = 0;
    // 创建消息队列
    mqd_t mqd = mq_open(mq_name, O_RDWR | O_CREAT, 0644, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // 父子进程通过消息队列通信

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // 子进程接收
        char recv_buf[100];
        struct timespec time_info;
        for (size_t i = 0; i < 10; i++)
        {
            // 清空接收buf
            memset(recv_buf, 0, sizeof(recv_buf));
            // 设置接收等待时间
            clock_gettime(CLOCK_REALTIME, &time_info);
            time_info.tv_sec += 5;

            // 接收消息队列的数据，打印到控制台
            if (mq_timedreceive(mqd, recv_buf, sizeof(recv_buf), 0, &time_info) == -1)
            {
                perror("mq_timedreceive");
            }
            printf("子进程接收到数据：%s\n", recv_buf);
        }
    }
    else
    {
        // 父进程发送数据到消息队列
        char send_buf[100];
        struct timespec time_info;

        for (size_t i = 0; i < 10; i++)
        {
            // 清空buf
            memset(send_buf, 0, 100);
            clock_gettime(CLOCK_REALTIME, &time_info);
            // printf("current time: %ld s \n", (long int)time_info.tv_sec);
            time_info.tv_sec += 5;
            sprintf(send_buf, "父进程第%d次发送消息\n", (int)(i + 1));
            if ((mq_timedsend(mqd, send_buf, strlen(send_buf), 0, &time_info)) != 0)
            {
                perror("mq_timedsend");
            }
            printf("父进程发送第%d条消息，休息1s\n", (int)(i + 1));
            sleep(1);
        }
    }

    close(mqd);
    // 释放消息队列
    if (pid > 0)
    {
        mq_unlink(mq_name);
    }

    return 0;
}
