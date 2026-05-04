#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_COUNT 20000

/**
 * @brief 对传入值累加 1
 *
 * @param argv 传入指针
 * @return void* 无返回值
 */
void *add_thread(void *argv)
{
    int *p = (int *)argv;
    (*p)++;
    return (void *)0;
}

int main(void)
{

    pthread_t tid[THREAD_COUNT];

    int num = 0;
    // 创建线程进行累加
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&tid[i], NULL, add_thread, &num);
    }
    // 等待线程结束
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(tid[i], NULL);
    }

    printf("累加结果：%d\n", num); // 预期结果不确定，多个线程出现竞态条件

    return 0;
}
