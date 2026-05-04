#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 5
#define TOTAL_ITEMS 10

sem_t *full;  // 缓冲区已占用槽位数
sem_t *empty; // 缓冲区空闲槽位数

// 环形缓冲区及读写索引
int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

int rand_num()
{
    srand(time(NULL));
    return rand() % 100;
}

void *producer(void *argv)
{
    // 生产者：持续生产数据填入缓冲区
    (void)argv;
    for (int i = 0; i < TOTAL_ITEMS; i++)
    {
        sem_wait(empty);

        // 模拟生产耗时
        sleep(1);
        buffer[in] = rand_num();

        int empty_val, full_val;
        sem_getvalue(empty, &empty_val);
        sem_getvalue(full, &full_val);
        printf("[生产] 写入buffer[%d]=%d  empty=%d, full=%d\n",
               in, buffer[in], empty_val, full_val);

        in = (in + 1) % BUFFER_SIZE;
        sem_post(full);
    }
    return (void *)0;
}

void *consumer(void *argv)
{
    // 消费者：持续从缓冲区取出数据处理
    (void)argv;
    for (int i = 0; i < TOTAL_ITEMS; i++)
    {
        sem_wait(full);

        int empty_val, full_val;
        sem_getvalue(empty, &empty_val);
        sem_getvalue(full, &full_val);
        printf("[消费] 读取buffer[%d]=%d  empty=%d, full=%d\n",
               out, buffer[out], empty_val, full_val);

        out = (out + 1) % BUFFER_SIZE;
        sem_post(empty);

        // 模拟消费耗时
        sleep(2);
    }
    return (void *)0;
}

int main()
{
    full = malloc(sizeof(sem_t));
    empty = malloc(sizeof(sem_t));

    // 初始化计数信号量
    // empty初始值=5: 缓冲区全部空闲，可取值0~5
    // full初始值=0: 缓冲区无数据，可取值0~5
    // 对比二值信号量只能取0~1，计数信号量可累积到大于1的值
    sem_init(full, 0, 0);
    sem_init(empty, 0, BUFFER_SIZE);

    pthread_t producer_thread, consumer_thread;
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    sem_destroy(full);
    sem_destroy(empty);
    free(full);
    free(empty);

    return 0;
}
