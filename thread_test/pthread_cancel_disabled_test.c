/**
 * @file pthread_cancel_disabled_test.c
 * @brief 线程取消 —— 禁用取消模式（临界区保护）
 *
 * 知识点：
 *   - PTHREAD_CANCEL_DISABLE：禁用取消请求
 *   - pthread_setcancelstate() 设置取消状态
 *   - 在临界区中禁用取消，防止资源泄漏或死锁
 *   - pthread_testcancel() 手动插入取消点
 *
 * 编译：gcc pthread_cancel_disabled_test.c -o pthread_cancel_disabled_test -lpthread
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static FILE *shared_fp = NULL;

/**
 * @brief 清理函数
 */
void cleanup_file(void *arg)
{
    (void)arg;
    printf("[清理函数] 关闭文件并释放资源\n");
    if (shared_fp)
    {
        fclose(shared_fp);
        shared_fp = NULL;
    }
}

/**
 * @brief 演示线程：在临界区中禁用取消
 *
 * 关键模式：
 *   1. 进入临界区前：禁用取消（pthread_setcancelstate DISABLE）
 *   2. 执行临界区操作（安全，不会被取消打断）
 *   3. 退出临界区后：恢复取消（pthread_setcancelstate ENABLE）
 *   4. 手动检查取消请求（pthread_testcancel）
 */
void *protected_thread(void *arg)
{
    (void)arg;
    printf("[受保护线程] 启动\n");

    pthread_cleanup_push(cleanup_file, NULL);

    for (int round = 1; round <= 5; round++)
    {
        /*********************************************************************
         * 临界区开始 —— 禁用取消
         *
         * 如果在 fopen/fclose 之间被取消，文件可能损坏或资源泄漏。
         * 因此在临界区中禁用取消。
         *********************************************************************/
        int oldstate;
        /**
         * @fn pthread_setcancelstate(int state, int *oldstate)
         * @brief 设置当前线程的取消状态
         * @param state    取消状态：
         *                 - PTHREAD_CANCEL_ENABLE   允许取消（默认）
         *                 - PTHREAD_CANCEL_DISABLE  禁用取消（请求挂起）
         * @param oldstate [输出] 保存旧状态，不需要传 NULL
         * @return 成功返回0，失败返回错误号
         * @note 禁用取消时，cancel 请求不会丢失，而是挂起(pending)，
         *       等下次启用取消后，在下一个取消点生效。
         */
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
        printf("[受保护线程] 第%d轮: 临界区开始（取消已禁用）\n", round);

        // ---- 临界区操作：打开文件、写入、关闭 ----
        shared_fp = fopen("/tmp/cancel_test.txt", "a");
        if (shared_fp)
        {
            fprintf(shared_fp, "第%d轮写入\n", round);
            fflush(shared_fp);
            fclose(shared_fp);
            shared_fp = NULL;
            printf("[受保护线程] 第%d轮: 文件写入完成\n", round);
        }

        // ---- 临界区结束：恢复取消状态 ----
        pthread_setcancelstate(oldstate, NULL);
        printf("[受保护线程] 第%d轮: 临界区结束（取消已恢复）\n", round);

        /**
         * @fn pthread_testcancel(void)
         * @brief 手动创建一个取消点
         * @note 恢复取消状态后，立即检查是否有挂起的取消请求。
         *       如果有，线程在此处被取消。
         *       这比依赖 sleep 等取消点更精确可控。
         */
        pthread_testcancel(); // 检查挂起的取消请求

        // 模拟非临界区工作
        printf("[受保护线程] 第%d轮: 非临界区工作（sleep 是取消点）...\n", round);
        sleep(1); // 这里也是取消点
    }

    pthread_cleanup_pop(0);
    printf("[受保护线程] 正常结束\n");
    return (void *)0xDEAD;
}

int main(void)
{
    pthread_t tid;

    printf("===== 线程取消状态演示：禁用取消 =====\n\n");

    pthread_create(&tid, NULL, protected_thread, NULL);

    // 让线程运行到第2轮的临界区中
    printf("[主线程] 等待1秒后发送取消请求（线程可能在临界区中）...\n");
    sleep(1);

    printf("[主线程] 发送取消请求!\n");
    pthread_cancel(tid);

    /**
     * 取消请求被挂起，直到线程退出临界区并恢复取消状态后，
     * 在 pthread_testcancel() 或 sleep() 处才真正被取消。
     */
    void *retval = NULL;
    pthread_join(tid, &retval);

    if (retval == PTHREAD_CANCELED)
    {
        printf("[主线程] 线程已被取消\n");
    }
    else
    {
        printf("[主线程] 线程正常结束，返回值=%p\n", retval);
    }

    // 验证文件是否完整写入
    printf("\n===== 验证文件内容 =====\n");
    printf("[主线程] /tmp/cancel_test.txt 内容:\n");
    FILE *fp = fopen("/tmp/cancel_test.txt", "r");
    if (fp)
    {
        char line[64];
        while (fgets(line, sizeof(line), fp))
        {
            printf("  %s", line);
        }
        fclose(fp);
    }

    /**************************************************************************
     * 取消状态总结：
     *
     * 状态              | 取消请求行为                | 使用场景
     * ----------------- | --------------------------- | --------------------
     * ENABLE（默认）    | 到达取消点时生效            | 一般代码
     * DISABLE           | 请求挂起，不生效            | 临界区（锁、文件、内存）
     *
     * 取消类型（与状态组合使用）：
     * 类型              | 生效条件                    | 安全性
     * ----------------- | --------------------------- | ------
     * DEFERRED（默认）  | 到达取消点函数              | 安全
     * ASYNCHRONOUS      | 任意时刻                    | 危险
     *
     * 推荐的临界区保护模式：
     *   pthread_setcancelstate(DISABLE, &old);  // 禁用
     *   ... 临界区操作 ...
     *   pthread_setcancelstate(old, NULL);       // 恢复
     *   pthread_testcancel();                    // 立即检查挂起请求
     **************************************************************************/

    printf("\n程序正常退出\n");
    return 0;
}
