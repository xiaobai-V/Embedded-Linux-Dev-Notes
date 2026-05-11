#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h> // 补充 strerror 函数需要的头文件

// 线程执行函数
void *thread_function(void *arg)
{
    // 动态分配内存存储返回值
    int *result = (int *)malloc(sizeof(int));
    *result = 42;
    return (void *)result;
}

int main()
{
    pthread_t tid;
    void *thread_result;

    // 创建线程
    int ret = pthread_create(&tid, NULL, thread_function, NULL);
    if (ret != 0)
    {
        printf("线程创建失败: %s\n", strerror(ret));
        return 1;
    }

    // 等待线程结束并获取返回值
    ret = pthread_join(tid, &thread_result);
    if (ret != 0)
    {
        printf("等待线程失败: %s\n", strerror(ret));
        return 1;
    }

    // 解析线程返回值并打印
    int *result = (int *)thread_result;
    printf("子线程返回值: %d\n", *result);

    // 释放动态分配的内存
    free(result);

    return 0;
}