#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_COUNT 20000

// static: 静态变量，作用域限制在当前文件内
// pthread_mutex_t: POSIX线程库的互斥锁类型
// PTHREAD_MUTEX_INITIALIZER: 静态初始化宏，用于编译时初始化互斥锁
static pthread_mutex_t conter_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 对传入值累加 1
 *
 * @param argv 传入指针
 * @return void* 无返回值
 */
void *add_thread(void *argv)
{
    int *p = (int *)argv;
    /**
     * @brief 简述：加锁
     *
     * @def 函数原型：pthread_mutex_lock(pthread_mutex_t *mutex);
     *
     * @param mutex：
     *      含义：要加锁的互斥锁指针
     *
     * @return 返回值：
     *      成功：0
     *      失败：非0
     * @remark 注意事项/易错点：
     */
    pthread_mutex_lock(&conter_mutex); // 加锁
    (*p)++;
    pthread_mutex_unlock(&conter_mutex); // 解锁
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

static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 对传入值累加 1
 *
 * @param argv 传入指针
 * @return void* 无返回值
 */
void *add_thread(void *argv)
{
    int *p = (int *)argv;
    /**
     * @brief 简述：加锁
     *
     * @def 函数原型：pthread_mutex_lock(pthread_mutex_t *mutex);
     *
     * @param mutex：
     *      含义：要加锁的互斥锁指针
     *
     * @return 返回值：
     *      成功：0
     *      失败：非0
     * @remark 注意事项/易错点：
     */
    pthread_mutex_lock(&counter_mutex); // 加锁
    (*p)++;
    pthread_mutex_unlock(&counter_mutex); // 解锁
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
