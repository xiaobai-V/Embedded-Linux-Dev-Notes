/**
 * @file pthread_detach_test.c
 * @brief 线程分离（detach）—— 自动回收线程资源
 *
 * 知识点：
 *   - 可 joinable（默认）vs detached 线程的区别
 *   - pthread_detach() 将线程设置为分离态
 *   - 分离线程终止后系统自动回收资源，不需要也不能 pthread_join
 *
 * 编译：gcc pthread_detach_test.c -o pthread_detach_test -lpthread
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief 分离线程的工作函数：模拟后台任务
 *
 * @param arg 未使用
 * @return NULL（分离线程的返回值无法被 join 获取，无人接收）
 */
void *background_task(void *arg)
{
    (void)arg;

    // 获取自身线程ID
    pthread_t self = pthread_self();

    /**
     * @fn pthread_self(void)
     * @brief 获取当前线程的标识符（类似 getpid()）
     * @return 当前线程的 pthread_t
     */
    printf("[分离线程] 启动，tid=%lu\n", (unsigned long)self);

    for (int i = 1; i <= 5; i++)
    {
        printf("[分离线程] 执行中... (%d/5)\n", i);
        sleep(1);
    }

    printf("[分离线程] 执行完毕，资源将由系统自动回收\n");
    return NULL;
}

/**
 * @brief 在线程内部自我分离
 *
 * 演示第二种分离方式：线程自己调用 pthread_detach(pthread_self())
 * 效果等价于在创建后由主线程调用 pthread_detach(tid)
 */
void *self_detach_task(void *arg)
{
    (void)arg;

    // 线程内部自我分离
    pthread_detach(pthread_self());

    printf("[自分离线程] 已将自己设置为分离态\n");
    sleep(2);
    printf("[自分离线程] 工作完成\n");
    return NULL;
}

int main(void)
{
    pthread_t tid1, tid2;

    /**************************************************************************
     * 线程的两种状态：
     *
     * 状态            | 创建方式              | 资源回收      | 可否 join
     * --------------- | --------------------- | ------------- | ---------
     * Joinable（默认）| pthread_create 默认    | 必须 join     | 可以
     * Detached        | pthread_detach 转换    | 系统自动回收  | 不可以
     *
     * 类比进程：
     *   Joinable  → 必须 waitpid() 回收的子进程
     *   Detached  → 不需要等待的守护进程
     **************************************************************************/

    /**************************************************************************
     * 演示1：创建后由主线程调用 pthread_detach
     **************************************************************************/
    printf("===== 演示1：主线程 detach 子线程 =====\n");

    pthread_create(&tid1, NULL, background_task, NULL);

    /**
     * @fn pthread_detach(pthread_t thread)
     * @brief 将指定线程标记为分离态
     * @param thread 目标线程标识符
     * @return 成功返回0，失败返回错误号
     * @note 一旦 detach，该线程终止时系统自动回收资源
     *       不能再对已 detach 的线程调用 pthread_join（会返回 EINVAL）
     */
    pthread_detach(tid1);

    // 对已 detach 的线程 join 会失败
    int ret = pthread_join(tid1, NULL);
    if (ret != 0)
    {
        printf("[主线程] pthread_join 失败: %s（预期行为，已 detach 的线程不能 join）\n",
               strerror(ret));
    }

    /**************************************************************************
     * 演示2：线程内部自我分离
     **************************************************************************/
    printf("\n===== 演示2：线程内部自我 detach =====\n");

    pthread_create(&tid2, NULL, self_detach_task, NULL);

    // 不需要也不应该 join tid2

    /**************************************************************************
     * 等待分离线程执行完毕（仅为了演示输出，实际项目不需要等待）
     *
     * 注意：这里用 sleep 仅为演示，生产代码不应依赖 sleep 同步。
     * 正确做法：使用 pthread_join（joinable）或 条件变量通知。
     **************************************************************************/
    printf("[主线程] 等待分离线程完成（sleep 仅演示用）...\n");
    sleep(7); // pthread_detach不会等待子进程结束，如果再子线程执行完毕之前主线程退出，子线程会被强制终止，
    // 因此需要等待足够的时间确保子线程完成自己的任务

    printf("\n程序正常退出\n");
    return 0;
}
