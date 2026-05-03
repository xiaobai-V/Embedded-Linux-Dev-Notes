/**
 * @file create_test.c
 * @brief 多线程环形缓冲区 —— 生产者/消费者模型
 *
 * 演示 POSIX Threads (pthread) 核心API：
 *   - pthread_create   创建线程
 *   - pthread_join     等待线程结束
 *   - pthread_mutex_t  互斥锁（保护共享资源）
 *   - pthread_cond_t   条件变量（线程间等待/唤醒通知）
 *   - pthread_cancel   请求取消线程（本示例改用标志位优雅退出）
 *
 * 编译：gcc create_test.c -o create_test -lpthread
 * 运行：./create_test，键入文字后回车可看到回显，按 Ctrl+D 退出
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_LEN 32 // 环形缓冲区大小（实际可用空间为 BUF_LEN-1）

/*******************************************************************************
 * 环形缓冲区核心变量
 ******************************************************************************/
static int head = 0; // 写指针：下一个写入的位置
static int tail = 0; // 读指针：下一个读取的位置
static char buffer[BUF_LEN];

/*******************************************************************************
 * 线程同步原语
 *
 * @def pthread_mutex_t
 * @brief 互斥锁（Mutex = Mutual Exclusion）
 *         用于保护共享资源的互斥访问，同一时刻只允许一个线程持有锁。
 *         常用API：
 *           pthread_mutex_init()    — 初始化锁
 *           pthread_mutex_lock()    — 加锁（阻塞式，锁被占有时阻塞等待）
 *           pthread_mutex_trylock() — 尝试加锁（非阻塞，失败立即返回EBUSY）
 *           pthread_mutex_unlock()  — 解锁
 *           pthread_mutex_destroy() — 销毁锁（释放系统资源）
 *
 * @def pthread_cond_t
 * @brief 条件变量（Condition Variable）
 *         用于线程间的等待/唤醒通知，必须与互斥锁配合使用。
 *         典型用法：
 *           等待方：lock → while(!condition) wait(cond, mutex) → 处理 → unlock
 *           唤醒方：lock → 修改条件 → signal/broadcast(cond) → unlock
 *         常用API：
 *           pthread_cond_init()      — 初始化条件变量
 *           pthread_cond_wait()      — 阻塞等待通知（自动释放并重新获取锁）
 *           pthread_cond_signal()    — 唤醒至少一个等待线程
 *           pthread_cond_broadcast() — 唤醒所有等待线程
 *           pthread_cond_destroy()   — 销毁条件变量
 *
 * @note 为什么用 while 而不是 if 来检查条件？
 *       1. 虚假唤醒（Spurious Wakeup）：POSIX 允许 pthread_cond_wait()
 *          在没有 signal 的情况下返回，必须重新检查条件。
 *       2. 多线程竞争：如果有多个等待者，被唤醒时条件可能已被其他线程改变。
 ******************************************************************************/
static pthread_mutex_t mutex;
static pthread_cond_t cond_not_empty; // 缓冲区非空：唤醒读线程
static pthread_cond_t cond_not_full;  // 缓冲区非满：唤醒写线程

/*******************************************************************************
 * 退出控制
 *
 * 使用原子标志位实现优雅退出，避免 pthread_cancel 的异步终止风险。
 * volatile 确保编译器不会将对此变量的读取优化为缓存值。
 ******************************************************************************/
static volatile int running = 1;

// 初始化缓冲区 + 线程同步工具
void initBuf(void)
{
    head = 0;
    tail = 0;
    memset(buffer, 0, BUF_LEN);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_not_empty, NULL);
    pthread_cond_init(&cond_not_full, NULL);
}

// 销毁锁和条件变量
void destroyBuf(void)
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_not_empty);
    pthread_cond_destroy(&cond_not_full);
}

/*************************
 环形缓冲区核心工具函数
 1. 判空：tail == head
 2. 判满：(head + 1) % BUF_LEN == tail（牺牲1字节区分空/满）
**************************/
static int isBufEmpty(void) { return tail == head; }
static int isBufFull(void) { return (head + 1) % BUF_LEN == tail; }

/**
 * @brief 线程安全写入环形缓冲区（阻塞式）
 * @param ch 要写入的字节
 * @return 成功写入返回1，线程应退出时返回0
 */
int writeBuf(char const ch)
{
    pthread_mutex_lock(&mutex); // 加锁保护共享资源

    // 缓冲区满 → 阻塞等待，直到有空闲空间
    while (isBufFull() && running)
    {
        pthread_cond_wait(&cond_not_full, &mutex);
    }

    // 检查退出标志
    if (!running)
    {
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    // 写入数据
    buffer[head] = ch;
    head = (head + 1) % BUF_LEN;

    pthread_cond_signal(&cond_not_empty); // 唤醒读线程：有数据了
    pthread_mutex_unlock(&mutex);         // 解锁

    return 1;
}

/**
 * @brief 线程安全读取环形缓冲区（阻塞式）
 * @param ch 输出参数，读取的字节存入此处
 * @return 成功读取返回1，线程应退出时返回0
 */
int readBuf(char *ch)
{
    pthread_mutex_lock(&mutex);

    // 缓冲区空 → 阻塞等待，直到有数据
    while (isBufEmpty() && running)
    {
        pthread_cond_wait(&cond_not_empty, &mutex);
    }

    // 退出条件：标志位已清除且缓冲区已空
    if (!running && isBufEmpty())
    {
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    // 读取数据
    *ch = buffer[tail];
    tail = (tail + 1) % BUF_LEN;

    pthread_cond_signal(&cond_not_full); // 唤醒写线程：有空间了
    pthread_mutex_unlock(&mutex);

    return 1;
}

// 输入线程：从键盘读取数据 → 写入环形缓冲区
void *input_thread(void *arg)
{
    (void)arg;
    int ch;
    while (1)
    {
        ch = fgetc(stdin);
        if (ch == EOF)
        { // 按下 Ctrl+D 退出
            printf("\n输入结束，线程退出\n");
            break;
        }
        if (!writeBuf((char)ch))
        {
            break; // 缓冲区已关闭
        }
    }
    return NULL;
}

// 输出线程：从环形缓冲区读取 → 打印到终端
void *output_thread(void *arg)
{
    (void)arg;
    char ch;
    while (readBuf(&ch))
    {
        printf("%c", ch);
        fflush(stdout); // 强制刷新输出，避免缓冲延迟
    }
    return NULL;
}

/**
 * @brief 主函数 —— 演示线程创建、等待与退出
 *
 * @fn pthread_create(pthread_t *thread, const pthread_attr_t *attr,
 *                     void *(*start_routine)(void *), void *arg)
 * @brief 创建新线程
 * @param thread        输出参数，成功后存储新线程的标识符
 * @param attr          线程属性（NULL = 默认属性：可 joinable、默认栈大小）
 * @param start_routine 线程入口函数，签名为 void *(*)(void *)
 * @param arg           传递给入口函数的参数
 * @return 成功返回0，失败返回错误号（errno 值，不设置 errno 本身）
 *
 * @fn pthread_join(pthread_t thread, void **retval)
 * @brief 阻塞等待指定线程结束，并回收其资源（类似 waitpid）
 * @param thread  目标线程标识符
 * @param retval  输出参数，接收线程的返回值（pthread_exit 的参数或 return 值）
 *               不关心可传 NULL
 * @return 成功返回0，失败返回错误号
 * @note 必须对每个可 joinable 的线程调用 pthread_join，否则造成资源泄漏
 *       （类似僵尸进程）。每个线程只能被 join 一次。
 *
 * @fn pthread_cancel(pthread_t thread)
 * @brief 请求取消指定线程（发送取消请求，非强制终止）
 * @param thread 目标线程标识符
 * @return 成功返回0，失败返回错误号
 * @note 线程被取消的时机取决于其取消类型：
 *       - PTHREAD_CANCEL_DEFERRED（默认）：延迟到下一个取消点才生效
 *         取消点包括：pthread_cond_wait, read, write, printf, sleep 等
 *       - PTHREAD_CANCEL_ASYNCHRONOUS：可随时被取消（危险，不推荐）
 *       本示例改用标志位退出，避免 cancel 导致的资源清理不确定性。
 ******************************************************************************/
int main(void)
{
    /**
     * @brief 线程标识符
     * pthread_t 是 POSIX 线程的标识类型（通常为 unsigned long 或结构体），
     * 用于后续的 join/cancel 等操作。注意：这是线程 ID，不是进程 PID。
     */
    pthread_t tid_input, tid_output;

    // 初始化
    initBuf();

    // 创建线程
    if (pthread_create(&tid_input, NULL, input_thread, NULL) != 0)
    {
        perror("输入线程创建失败");
        destroyBuf();
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&tid_output, NULL, output_thread, NULL) != 0)
    {
        perror("输出线程创建失败");
        running = 0;
        pthread_cond_signal(&cond_not_full); // 唤醒可能阻塞的输入线程
        pthread_join(tid_input, NULL);       // 回收已创建的输入线程
        destroyBuf();
        exit(EXIT_FAILURE);
    }

    // 等待输入线程结束（用户按 Ctrl+D）
    pthread_join(tid_input, NULL);

    // 输入线程退出后，通知输出线程退出
    pthread_mutex_lock(&mutex);
    running = 0;                             // 设置退出标志
    pthread_cond_broadcast(&cond_not_empty); // 唤醒可能在等待数据的输出线程
    pthread_cond_broadcast(&cond_not_full);  // 唤醒可能在等待空间的输入线程
    pthread_mutex_unlock(&mutex);

    // 等待输出线程优雅退出
    pthread_join(tid_output, NULL);

    // 释放资源
    destroyBuf();
    printf("程序正常退出\n");
    return 0;
}
