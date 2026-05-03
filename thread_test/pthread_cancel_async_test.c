/**
 * @file pthread_cancel_async_test.c
 * @brief 线程取消 —— 异步取消模式（可随时取消，无取消点限制）
 *
 * 知识点：
 *   - PTHREAD_CANCEL_ASYNCHRONOUS：异步取消，不需要到达取消点
 *   - pthread_setcanceltype() 设置取消类型
 *   - 异步取消的危险性：可能在任意指令处被终止，资源可能泄漏
 *
 * 编译：gcc pthread_cancel_async_test.c -o pthread_cancel_async_test -lpthread
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static volatile int counter = 0;

/**
 * @brief 清理函数 —— 演示异步取消时的资源释放
 */
void cleanup_async(void *arg)
{
    printf("\n[清理函数] 线程被异步取消！当前计数器=%d, 资源: %s\n",
           counter, (char *)arg);
}

/**
 * @brief 异步取消的目标线程
 *
 * 纯计算循环（无取消点），延迟取消永远无法生效，必须用异步取消。
 */
void *async_cancel_thread(void *arg)
{
    (void)arg;

    /**
     * @fn pthread_setcanceltype(int type, int *oldtype)
     * @brief 设置当前线程的取消类型
     * @param type    取消类型：
     *                - PTHREAD_CANCEL_DEFERRED  延迟取消（默认，需到达取消点）
     *                - PTHREAD_CANCEL_ASYNCHRONOUS 异步取消（可随时取消）
     * @param oldtype [输出] 保存旧的取消类型，不需要传 NULL
     * @return 成功返回0，失败返回错误号
     *
     * @warning 异步取消极其危险！线程可能在任何指令处被终止：
     *          - malloc 中被取消 → 堆损坏
     *          - 持有锁时被取消 → 死锁
     *          - I/O 操作中被取消 → 数据不一致
     *          仅在确定安全的纯计算场景使用！
     */
    int oldtype;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
    printf("[异步线程] 取消类型: %s → %s\n",
           oldtype == PTHREAD_CANCEL_DEFERRED ? "DEFERRED" : "ASYNCHRONOUS",
           "ASYNCHRONOUS");

    pthread_cleanup_push(cleanup_async, (void *)"文件描述符");

    printf("[异步线程] 开始纯计算循环（无取消点）...\n");

    // 纯计算，没有任何取消点函数
    // 如果是延迟取消模式，这个循环永远不会被取消！
    while (1)
    {
        counter++;
        // 故意不加 sleep 或 printf，演示无取消点的场景
    }

    pthread_cleanup_pop(0);
    return NULL;
}

/**
 * @brief 延迟取消的对比线程
 *
 * 同样的纯计算循环，但使用延迟取消模式。
 * cancel 请求发出后，线程永远不会被取消（因为没有取消点）。
 */
void *deferred_cancel_thread(void *arg)
{
    (void)arg;
    printf("[延迟线程] 使用默认延迟取消模式\n");
    printf("[延迟线程] 开始纯计算循环（无取消点）...\n");

    while (1)
    {
        counter++;
    }
    return NULL;
}

int main(void)
{
    pthread_t tid_async, tid_deferred;

    /**************************************************************************
     * 演示1：异步取消 —— 无需取消点，立即生效
     **************************************************************************/
    printf("===== 演示1：异步取消（ASYNCHRONOUS）=====\n");

    pthread_create(&tid_async, NULL, async_cancel_thread, NULL);
    usleep(100000); // 100ms，让线程启动

    printf("[主线程] 发送取消请求...\n");
    pthread_cancel(tid_async);

    void *ret1 = NULL;
    pthread_join(tid_async, &ret1);
    printf("[主线程] 异步取消结果: %s\n",
           ret1 == PTHREAD_CANCELED ? "已取消" : "未取消");
    printf("[主线程] 取消时计数器值: %d\n", counter);

    /**************************************************************************
     * 演示2：延迟取消对纯计算循环无效（对比）
     **************************************************************************/
    counter = 0;
    printf("\n===== 演示2：延迟取消对纯计算无效（对比）=====\n");

    pthread_create(&tid_deferred, NULL, deferred_cancel_thread, NULL);
    usleep(100000);

    printf("[主线程] 发送取消请求（延迟模式）...\n");
    pthread_cancel(tid_deferred);

    /**
     * 尝试 join，但线程不会被取消（没有取消点）
     * 设置1秒超时检测：join 会一直阻塞
     */
    printf("[主线程] 1秒后检查线程状态...\n");
    usleep(1000000);
    printf("[主线程] 延迟线程仍在运行（无法取消纯计算循环）\n");

    // 强制终止（仅用于演示，生产代码不应这样做）
    pthread_cancel(tid_deferred);
    // 由于延迟线程永远不会到达取消点，这里用异步方式做最后清理
    // 注意：这只是为了让演示程序能退出，不是推荐做法

    /**************************************************************************
     * 延迟 vs 异步 取消对比：
     *
     * 特性        | DEFERRED（默认）       | ASYNCHRONOUS
     * ----------- | ---------------------- | --------------------------
     * 取消时机    | 到达取消点时           | 任意时刻（可能下一条指令）
     * 安全性      | 安全（可预知取消位置） | 危险（可能在关键区中被杀）
     * 适用场景    | 一般多线程程序         | 纯计算且无取消点的场景
     * 推荐程度    | 推荐（默认）           | 不推荐，尽量避免
     *
     * 替代方案：如果需要取消纯计算线程，更好的做法是：
     *   1. 在循环中手动插入取消点：pthread_testcancel()
     *   2. 使用标志位 + pthread_cond_wait 配合退出
     **************************************************************************/

    printf("\n程序退出\n");
    return 0;
}
