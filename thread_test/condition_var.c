#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int count = 0; // 缓冲区当前元素数量

// 初始化互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// 初始化条件变量
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    int item = 0;
    while (1)
    {
        // 获取互斥锁
        pthread_mutex_lock(&mutex); // 阻塞等待
        // 等待缓冲区有空间
        while (count >= BUFFER_SIZE)
        {
            printf("缓冲区已满，生产者等待\n");
            pthread_cond_wait(&cond, &mutex);
        }
        // 生产一个元素
        buffer[count] = item++;
        printf("生产者生产了 %d\n", buffer[count]);
        count++;
        // 通知消费者有新元素
        pthread_cond_signal(&cond);
        // 释放互斥锁
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg)
{
    while (1)
    {
        // 获取互斥锁
        pthread_mutex_lock(&mutex); // 阻塞等待
        // 等待缓冲区有元素
        while (count <= 0)
        {
            printf("缓冲区为空，消费者等待\n");
            pthread_cond_wait(&cond, &mutex);
        }

        // 消费一个元素
        int item = buffer[count - 1];
        count--;
        printf("消费者消费了 %d\n", item);
        // 通知生产者有空间
        pthread_cond_signal(&cond);
        // 释放互斥锁
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char const *argv[])
{
    // 创建两个线程
    pthread_t producer_thread, consumer_thread;
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // 等待两个线程结束
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // 销毁互斥锁, 静态初始化的互斥锁不需要销毁
    // pthread_mutex_destroy(&mutex);
    // 销毁条件变量， 静态初始化的条件变量不需要销毁
    // pthread_cond_destroy(&cond);

    return 0;
}
