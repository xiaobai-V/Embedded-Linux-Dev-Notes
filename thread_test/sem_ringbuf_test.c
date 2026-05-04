/**
 * @file sem_ringbuf_test.c
 * @brief 信号量 + 环形缓冲区实现生产者-消费者模型
 *
 * 知识点：
 *   - POSIX 无名信号量：sem_init / sem_wait / sem_post / sem_destroy
 *   - 用三个信号量实现线程同步：
 *       sem_empty  — 空槽位数（初始值 = BUF_SIZE，控制生产者不溢出）
 *       sem_full   — 数据个数  （初始值 = 0，控制消费者不欠读）
 *       sem_mutex  — 互斥访问  （初始值 = 1，保护缓冲区结构）
 *   - 环形缓冲区（head/tail 双指针 + 取模运算）
 *
 * 对比条件变量方案（thread_ringbuf_test.c）：
 *   - 信号量本身就是计数器，不需要 while 循环检查条件
 *   - 信号量无需担心虚假唤醒问题
 *   - 代码更简洁，但灵活性略低于条件变量
 *
 * 编译：gcc sem_ringbuf_test.c -o sem_ringbuf_test -lpthread
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 5       // 环形缓冲区容量
#define ITEM_COUNT 20    // 总共生产的数据项数

/*******************************************************************************
 * 环形缓冲区
 ******************************************************************************/
static int buffer[BUF_SIZE];
static int head = 0; // 写入位置
static int tail = 0; // 读取位置

/*******************************************************************************
 * 三个信号量
 *
 * @def sem_t
 * @brief POSIX 无名信号量（Unnamed Semaphore）
 *         用于线程间同步（同一进程内），基于计数器实现。
 *         常用 API：
 *           sem_init(sem, pshared, value) — 初始化，pshared=0 表示线程间共享
 *           sem_wait(sem)                 — P 操作：计数器 -1，若 < 0 则阻塞
 *           sem_post(sem)                 — V 操作：计数器 +1，唤醒等待线程
 *           sem_destroy(sem)              — 销毁信号量，释放资源
 ******************************************************************************/
static sem_t sem_empty;  // 空槽位计数，初始 = BUF_SIZE
static sem_t sem_full;   // 数据项计数，初始 = 0
static sem_t sem_mutex;  // 互斥信号量，初始 = 1（二值信号量，等效 mutex）

/** 退出标志 */
static volatile int running = 1;

/**
 * @brief 生产者线程
 *
 * 同步流程：
 *   1. sem_wait(&sem_empty)  — 获取一个空槽位（空槽 -1，满则阻塞）
 *   2. sem_wait(&sem_mutex)  — 锁住缓冲区
 *   3. 写入数据
 *   4. sem_post(&sem_mutex)  — 解锁缓冲区
 *   5. sem_post(&sem_full)   — 增加一个数据项（数据 +1，唤醒消费者）
 *
 * @note sem_empty 和 sem_mutex 的 wait 顺序不能颠倒！
 *       如果先 lock 再 wait empty，缓冲区满时会死锁：
 *       生产者持锁等待空位，消费者等锁无法消费 → 死锁。
 */
void *producer(void *arg)
{
    (void)arg;

    for (int i = 1; i <= ITEM_COUNT; i++)
    {
        // 等待空槽位（P 操作）
        sem_wait(&sem_empty);

        // 锁住缓冲区
        sem_wait(&sem_mutex);

        // 写入环形缓冲区
        buffer[head] = i;
        printf("[生产者] 写入 buffer[%d] = %d\n", head, i);
        head = (head + 1) % BUF_SIZE;

        // 解锁缓冲区
        sem_post(&sem_mutex);

        // 通知消费者有新数据（V 操作）
        sem_post(&sem_full);

        // 模拟生产耗时
        usleep(100000); // 100ms
    }

    printf("[生产者] 生产完毕，共 %d 项\n", ITEM_COUNT);
    running = 0;

    // 唤醒消费者让其退出（防止消费者永远阻塞在 sem_wait）
    sem_post(&sem_full);

    return NULL;
}

/**
 * @brief 消费者线程
 *
 * 同步流程与生产者镜像对称：
 *   1. sem_wait(&sem_full)   — 获取一个数据项（数据 -1，空则阻塞）
 *   2. sem_wait(&sem_mutex)  — 锁住缓冲区
 *   3. 读取数据
 *   4. sem_post(&sem_mutex)  — 解锁缓冲区
 *   5. sem_post(&sem_empty)  — 增加一个空槽位（空槽 +1，唤醒生产者）
 */
void *consumer(void *arg)
{
    (void)arg;
    int consumed = 0;

    while (1)
    {
        // 等待数据项（P 操作）
        sem_wait(&sem_full);

        // 生产者已退出且无数据，消费者也退出
        if (!running && consumed >= ITEM_COUNT)
        {
            break;
        }

        // 锁住缓冲区
        sem_wait(&sem_mutex);

        // 从环形缓冲区读取
        int item = buffer[tail];
        printf("            [消费者] 读取 buffer[%d] = %d\n", tail, item);
        tail = (tail + 1) % BUF_SIZE;

        // 解锁缓冲区
        sem_post(&sem_mutex);

        // 通知生产者有空槽位（V 操作）
        sem_post(&sem_empty);

        consumed++;

        // 模拟消费耗时（比生产慢，容易观察到缓冲区满阻塞）
        usleep(200000); // 200ms
    }

    printf("[消费者] 消费完毕，共 %d 项\n", consumed);
    return NULL;
}

/**
 * @brief 主函数 —— 初始化信号量，创建线程，等待结束
 *
 * @note 信号量 vs 条件变量的选择：
 *       - 生产者-消费者这类"计数资源"问题，信号量更直观
 *       - 需要等待复杂条件（如 "A 且 B 且非 C"）时，条件变量更灵活
 */
int main(void)
{
    pthread_t tid_producer, tid_consumer;

    /**************************************************************************
     * 初始化三个信号量
     *
     * @fn sem_init(sem_t *sem, int pshared, unsigned int value)
     * @brief 初始化无名信号量
     * @param sem     信号量指针
     * @param pshared 0 = 线程间共享，非0 = 进程间共享
     * @param value   信号量初始值
     * @return 成功返回0，失败返回 -1 并设置 errno
     *
     * @note pshared=0 时信号量位于进程地址空间，仅同进程线程可用；
     *       pshared≠0 时信号量位于共享内存区域，可跨进程使用。
     *************************************************************************/
    sem_init(&sem_empty, 0, BUF_SIZE); // 初始空槽 = 缓冲区大小
    sem_init(&sem_full, 0, 0);         // 初始数据 = 0
    sem_init(&sem_mutex, 0, 1);        // 初始值 = 1（二值信号量 = 互斥锁）

    // 创建生产者和消费者线程
    pthread_create(&tid_producer, NULL, producer, NULL);
    pthread_create(&tid_consumer, NULL, consumer, NULL);

    // 等待线程结束
    pthread_join(tid_producer, NULL);
    pthread_join(tid_consumer, NULL);

    // 销毁信号量
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mutex);

    printf("\n===== 所有线程已完成 =====\n");

    return 0;
}
