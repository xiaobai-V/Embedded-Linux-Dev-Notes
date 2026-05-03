#define _POSIX_C_SOURCE 200112L

#include <time.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    // 打开消息队列
    char mq_name[] = "/p_c_mq";

    mqd_t mqdes = mq_open(mq_name, O_RDWR);
    if (mqdes < 0)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    struct timespec time_info;
    int timeout_count = 0; // 连续超时计数，防止生产者崩溃后死循环
    // 持续接收
    while (1)
    {
        char recv_buf[100];
        memset(recv_buf, 0, sizeof(recv_buf));
        // 从消息队列接收数据
        clock_gettime(CLOCK_REALTIME, &time_info);
        time_info.tv_sec += 5;

        ssize_t recv_num = mq_timedreceive(mqdes, recv_buf, sizeof(recv_buf), 0, &time_info);
        if (recv_num < 0)
        {
            // 超时（生产者可能已崩溃），累计3次超时则退出
            timeout_count++;
            perror("mq_timedreceive");
            if (timeout_count >= 3)
            {
                fprintf(stderr, "连续%d次超时，认为生产者已退出\n", timeout_count);
                close(mqdes);
                mq_unlink(mq_name);
                exit(EXIT_FAILURE);
            }
            continue;
        }
        timeout_count = 0; // 成功接收，重置计数

        // 读取数据
        if (recv_num == 0)
        {
            // 读取到空
            printf("生产者停止发送");
            close(mqdes);
            mq_unlink(mq_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            // 读取到数据
            printf("消费者接收到数据: %s\n", recv_buf);
        }
    }

    return 0;
}
