#define _POSIX_C_SOURCE 200112L

#include <time.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    // 创建消息队列
    char mq_name[] = "/p_c_mq";
    struct mq_attr attr;
    attr.mq_curmsgs = 0;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;

    mqd_t mqdes = mq_open(mq_name, O_CREAT | O_RDWR, 0644, &attr);
    if (mqdes == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    struct timespec time_info;
    // 持续发送
    while (1)
    {
        char send_buf[100];
        memset(send_buf, 0, sizeof(send_buf));
        // 从终端读取数据
        int read_num = read(STDIN_FILENO, send_buf, sizeof(send_buf));
        if (read_num == -1) // 读取失败
        {
            perror("read");
            continue;
        }

        clock_gettime(CLOCK_REALTIME, &time_info);
        time_info.tv_sec += 5;

        if (read_num > 0) // 读取到数据
        {
            // 用 read_num 而非 strlen：read 可能读到含 \0 的数据，strlen 会截断
            if (mq_timedsend(mqdes, send_buf, read_num, 0, &time_info) == -1)
            {
                perror("mq_timedsend");
            }
        }
        else if (read_num == 0) // 读取到EOF（Ctrl+D）
        {
            printf("生产者停止发送\n");
            // 发送0字节作为停止信号，consumer 的 recv_num==0 才能成立
            mq_timedsend(mqdes, "", 0, 0, &time_info);

            // 清理退出
            close(mqdes);
            // 消息队列由消费者清除
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
