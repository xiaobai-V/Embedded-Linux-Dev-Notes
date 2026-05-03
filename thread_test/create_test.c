/**
 * @file create_test.c
 * @brief 多线程基础示例 —— 无同步的简化版（教学用途）
 *
 * 演示 pthread 最基本的两个 API：
 *   - pthread_create() 创建线程
 *   - pthread_join()   等待线程结束
 *
 * 本示例故意省略了互斥锁和条件变量，展示无同步时的线程安全问题。
 * 完整版（线程安全）见 thread_ringbuf_test.c。
 *
 * 编译：gcc create_test.c -o create_test -lpthread
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_LEN 1024
char *buf;

/**
 * @brief 输入线程：从键盘读取字符 → 写入共享缓冲区
 *
 * @param argv 未使用（线程入口函数签名要求为 void *）
 * @return NULL（本线程为死循环，不会返回）
 *
 * @note 线程入口函数的固定签名：void *(*start_routine)(void *)
 *       返回值类型为 void *，参数为 void *，可传任意数据。
 */
void *input_thread(void *argv)
{
    int i = 0;
    while (1)
    {
        char c = fgetc(stdin);
        if (c && c != '\n')
        {
            buf[i++] = c; // 直接写入共享缓冲区（无锁保护，存在竞态）
        }

        if (i >= BUF_LEN)
        {
            i = 0;
        }
    }
}

/**
 * @brief 输出线程：从共享缓冲区读取字符 → 打印到终端
 *
 * 用 sleep(1) 轮询代替条件变量通知，属于忙等待的简化写法。
 * 实际项目应使用 pthread_cond_t 避免浪费 CPU。
 */
void *output_thread(void *)
{
    int i = 0;
    while (1)
    {
        if (buf[i])
        {

            fputc(buf[i], stdout);
            fputc('\n', stdout);
            buf[i++] = 0; // 读取后清零（无锁保护，存在竞态）
            if (i >= BUF_LEN)
            {
                i = 0;
            }
        }
        else
        {
            sleep(1); // 缓冲区空 → 休眠1秒再检查（忙等待）
        }
    }
}

int main(int argc, char const *argv[])
{
    pthread_t tid_input;  // 输入线程标识符
    pthread_t tid_output; // 输出线程标识符

    buf = malloc(BUF_LEN);
    memset(buf, 0, BUF_LEN);

    // 创建线程：成功返回0，失败返回非0
    /**
     * @fn pthread_create(pthread_t *thread, const pthread_attr_t *attr,
     *             void *(*start_routine)(void *), void *arg)
     * @brief 创建一个新线程
     * @param thread        [输出] 成功后存储新线程标识符
     * @param attr          线程属性，NULL = 默认（可joinable、默认栈大小8MB）
     * @param start_routine 线程入口函数，签名为 void *(*)(void *)
     * @param arg           传递给入口函数的参数，不需要时传 NULL
     * @return 成功返回0，失败返回错误号（不设置 errno）
     */
    pthread_create(&tid_input, NULL, input_thread, NULL);
    pthread_create(&tid_output, NULL, output_thread, NULL);

    /**
     * @fn pthread_join(pthread_t thread, void **retval)
     * @brief 阻塞等待指定线程结束，回收其资源
     * @param thread  目标线程标识符
     * @param retval  [输出] 接收线程返回值，不关心传 NULL
     * @return 成功返回0，失败返回错误号
     * @note 类似进程的 waitpid()，不调用会造成线程资源泄漏
     */
    pthread_join(tid_input, NULL);
    pthread_join(tid_output, NULL);

    // 释放堆内存
    free(buf);

    return 0;
}
