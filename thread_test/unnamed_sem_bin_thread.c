#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

sem_t unnamed_sem;
int shard_num = 0;

/**
 * 测试多线程同步
 * 多个线程同时对shared_num+1
 * 采用二值信号量充当互斥锁
 */
void *plusOne(void *argv)
{
    // 这里的二值信号量起到了互斥锁的作用，避免了线程间的竞态条件
    // 获取信号量
    sem_wait(&unnamed_sem); // -1

    (void)argv;
    int tmp = shard_num + 1; // 不是原子操作
    shard_num = tmp;

    // 释放信号量
    sem_post(&unnamed_sem); // +1

    return (void *)0;
}

int main()
{
    sem_init(&unnamed_sem, 0, 1);

    pthread_t tid[10000];
    for (int i = 0; i < 10000; i++)
    {
        pthread_create(tid + i, NULL, plusOne, NULL);
    }
    for (int i = 0; i < 10000; i++)
    {
        pthread_join(tid[i], NULL);
    }
    printf("shard_num is %d\n", shard_num); // 预期结果不确定，多个线程出现竞态条件
    // shard_num is 9999
    // shard_num is 9998

    sem_destroy(&unnamed_sem);

    return 0;
}