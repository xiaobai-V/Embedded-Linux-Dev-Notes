#include <stdio.h>
#include <pthread.h>
// 线程执行函数
void *thread_function(void *arg)
{
    int num = *(int *)arg;
    printf("子线程执行，参数为: %d\n", num);
    return NULL;
}
int main()
{
    pthread_t tid;
    int arg = 10;
    int ret = pthread_create(&tid, NULL, thread_function, &arg);
    if (ret != 0)
    {
        printf("线程创建失败: %s\n", strerror(ret));
        return 1;
    }
    printf("主线程继续执行\n");
    // 等待子线程结束
    pthread_join(tid, NULL);
    return 0;
}