#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

int shard_num = 0;
void *plusOne(void *argv)
{
    (void)argv;
    int tmp = shard_num + 1; // 不是原子操作
    shard_num = tmp;

    return (void *)0;
}

int main()
{
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

    return 0;
}