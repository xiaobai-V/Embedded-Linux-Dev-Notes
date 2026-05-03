/**
 * @file pthread_cancel_deferred_test.c
 * @brief 线程取消 —— 延迟取消模式（默认模式）
 *
 * 知识点：
 *   - pthread_cancel() 发送取消请求
 *   - PTHREAD_CANCEL_DEFERRED（默认）：延迟到取消点才生效
 *   - 常见取消点：pthread_cond_wait, read, write, printf, sleep, pause 等
 *   - pthread_cleanup_push / pop 注册取消时的清理函数
 *
 * 编译：gcc pthread_cancel_deferred_test.c -o pthread_cancel_deferred_test -lpthread
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief 清理函数 —— 线程被取消时自动调用（类似 atexit）
 *
 * @param arg 注册时传入的参数
 *
 * @note 清理函数的调用顺序：后注册的先调用（栈式 LIFO）
 *       正常退出（return）时如果 execute=1 也会调用
 */
void cleanup_handler(void *arg)
{
    printf("[清理函数] 正在释放资源: %s\n", (char *)arg);
}

/**
 * @brief 被取消的目标线程
 *
 * 演示：
 *   1. 注册清理函数（取消时自动调用）
 *   2. 在取消点（sleep）处被取消
 */
void *target_thread(void *arg)
{
    (void)arg;
    printf("[目标线程] 启动，即将进入循环\n");

    /**
     * @fn pthread_cleanup_push(void (*routine)(void *), void *arg)
     * @brief 注册清理函数到线程的清理栈（LIFO）
     * @param routine 清理函数指针
     * @param arg     传给清理函数的参数
     * @note 必须与 pthread_cleanup_pop 成对出现（它们是宏，{ } 配对）
     */

    /**
     * @fn pthread_cleanup_pop(int execute)
     * @brief 弹出最近注册的清理函数
     * @param execute 非0=立即执行该清理函数，0=仅弹出不清除
     */
    pthread_cleanup_push(cleanup_handler, (void *)"动态分配的内存");

    for (int i = 1; i <= 20; i++)
    {
        printf("[目标线程] 第 %d 次循环\n", i);
        /**
         * sleep() 是取消点！
         * 当其他线程调用 pthread_cancel() 后，线程不会立即终止，
         * 而是在执行到 sleep() 这个取消点时才真正被取消。
         */
        sleep(1); // ← 取消点：pthread_cancel 在这里生效
    }

    // 正常退出时，execute=1 也会执行清理函数
    pthread_cleanup_pop(1);

    printf("[目标线程] 正常结束（如果被取消，这行不会打印）\n");
    return NULL;
}

int main(void)
{
    pthread_t tid;

    printf("===== 线程取消演示：延迟取消（默认模式）=====\n\n");

    pthread_create(&tid, NULL, target_thread, NULL);

    // 让目标线程先运行3秒
    printf("[主线程] 等待3秒后发送取消请求...\n");
    sleep(3);

    /**
     * @fn pthread_cancel(pthread_t thread)
     * @brief 向指定线程发送取消请求
     * @param thread 目标线程标识符
     * @return 成功返回0（仅表示请求已发送，不代表线程已终止）
     * @note 取消不是立即终止，而是等到目标线程到达取消点才生效
     */
    printf("[主线程] 发送取消请求!\n");
    pthread_cancel(tid);

    /**
     * 被取消的线程，join 返回的值是 PTHREAD_CANCELED
     * 可以通过这个值判断线程是被取消还是正常退出
     */
    void *retval = NULL;
    pthread_join(tid, &retval);

    if (retval == PTHREAD_CANCELED)
    {
        printf("[主线程] 目标线程已被取消（返回值 == PTHREAD_CANCELED）\n");
    }
    else
    {
        printf("[主线程] 目标线程正常退出，返回值=%p\n", retval);
    }

    /**************************************************************************
     * 延迟取消要点总结：
     *
     * 1. 默认取消类型：PTHREAD_CANCEL_DEFERRED
     *    - cancel 请求不会立即生效
     *    - 线程到达"取消点"函数时才真正被取消
     *
     * 2. 常见取消点（man pthreads 查看完整列表）：
     *    - I/O 类:  read, write, printf, fgetc, fopen, fclose ...
     *    - 等待类:  sleep, usleep, pthread_cond_wait, pause, select ...
     *    - 注意：malloc, free 不是取消点
     *
     * 3. 被取消时的行为：
     *    - 先执行清理栈中的所有清理函数（LIFO 顺序）
     *    - 然后线程终止，返回值为 PTHREAD_CANCELED
     *
     * 4. 清理函数机制：
     *    pthread_cleanup_push(handler, arg)  — 注册（可多次，LIFO）
     *    pthread_cleanup_pop(execute)        — 弹出（execute!=0 则执行）
     *    线程被取消时：自动依次弹出并执行所有已注册的清理函数
     **************************************************************************/

    printf("\n程序正常退出\n");
    return 0;
}
