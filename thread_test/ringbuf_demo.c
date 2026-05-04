#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5     // 环形缓冲区可容纳的字符串数量
#define MAX_LINE_LEN 1024 // 每行最大长度

// 环形缓冲区结构
typedef struct
{
    char **buf;               // 存放字符串指针的数组
    int capacity;             // 缓冲区容量
    int read_idx;             // 读索引
    int write_idx;            // 写索引
    int count;                // 当前存储的字符串个数
    int input_done;           // 输入是否结束的标志
    pthread_mutex_t lock;     // 互斥锁
    pthread_cond_t not_full;  // 条件变量：缓冲区未满
    pthread_cond_t not_empty; // 条件变量：缓冲区非空
} ring_buffer_t;

// 初始化环形缓冲区
void rb_init(ring_buffer_t *rb, int capacity)
{
    rb->buf = (char **)malloc(capacity * sizeof(char *));
    rb->capacity = capacity;
    rb->read_idx = 0;
    rb->write_idx = 0;
    rb->count = 0;
    rb->input_done = 0;
    pthread_mutex_init(&rb->lock, NULL);
    pthread_cond_init(&rb->not_full, NULL);
    pthread_cond_init(&rb->not_empty, NULL);
}

// 销毁环形缓冲区，释放内部保存的所有字符串
void rb_destroy(ring_buffer_t *rb)
{
    // 释放所有尚未读取的字符串
    for (int i = 0; i < rb->count; i++)
    {
        int idx = (rb->read_idx + i) % rb->capacity;
        free(rb->buf[idx]);
    }
    free(rb->buf);
    pthread_mutex_destroy(&rb->lock);
    pthread_cond_destroy(&rb->not_full);
    pthread_cond_destroy(&rb->not_empty);
}

// 向环形缓冲区写入一个字符串（生产者调用）
// 若缓冲区满则阻塞，直到被输出线程取走数据
void rb_write(ring_buffer_t *rb, char *line)
{
    pthread_mutex_lock(&rb->lock);

    // 当缓冲区满且输入尚未结束时等待
    while (rb->count == rb->capacity && !rb->input_done)
    {
        pthread_cond_wait(&rb->not_full, &rb->lock);
    }

    // 如果输入已经结束，可能被通知退出，释放line并返回
    if (rb->input_done)
    {
        pthread_mutex_unlock(&rb->lock);
        free(line);
        return;
    }

    // 写入数据
    rb->buf[rb->write_idx] = line;
    rb->write_idx = (rb->write_idx + 1) % rb->capacity;
    rb->count++;

    // 通知输出线程有数据可读
    pthread_cond_signal(&rb->not_empty);
    pthread_mutex_unlock(&rb->lock);
}

// 从环形缓冲区读取一个字符串（消费者调用）
// 若缓冲区空且输入未结束则阻塞；若输入结束且缓冲区空则返回NULL
char *rb_read(ring_buffer_t *rb)
{
    pthread_mutex_lock(&rb->lock);

    // 当缓冲区空且输入未结束时等待
    while (rb->count == 0 && !rb->input_done)
    {
        pthread_cond_wait(&rb->not_empty, &rb->lock);
    }

    // 没有数据且输入已结束，返回NULL表示结束
    if (rb->count == 0)
    {
        pthread_mutex_unlock(&rb->lock);
        return NULL;
    }

    // 取出数据
    char *line = rb->buf[rb->read_idx];
    rb->read_idx = (rb->read_idx + 1) % rb->capacity;
    rb->count--;

    // 通知输入线程有空间可写
    pthread_cond_signal(&rb->not_full);
    pthread_mutex_unlock(&rb->lock);

    return line;
}

// 标记输入完成，唤醒可能阻塞的输出线程
void rb_set_input_done(ring_buffer_t *rb)
{
    pthread_mutex_lock(&rb->lock);
    rb->input_done = 1;
    // 唤醒可能在等待非空条件、或等待非满条件的线程
    pthread_cond_broadcast(&rb->not_empty);
    pthread_cond_broadcast(&rb->not_full);
    pthread_mutex_unlock(&rb->lock);
}

// 输入线程函数：读取键盘输入，逐行写入环形缓冲区
void *input_thread(void *arg)
{
    ring_buffer_t *rb = (ring_buffer_t *)arg;
    char line[MAX_LINE_LEN];

    printf("Input thread started. Type text (Ctrl+D to end):\n");
    while (fgets(line, sizeof(line), stdin))
    {
        // 去掉末尾换行符（如果存在）
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        // 动态分配内存保存这一行
        char *data = strdup(line);
        if (!data)
        {
            perror("strdup");
            break;
        }

        rb_write(rb, data);
    }

    // 读到EOF或出错，设置输入完成标志
    rb_set_input_done(rb);
    printf("Input thread finished.\n");
    return NULL;
}

// 输出线程函数：从环形缓冲区读取数据并打印
void *output_thread(void *arg)
{
    ring_buffer_t *rb = (ring_buffer_t *)arg;

    printf("Output thread started.\n");
    while (1)
    {
        char *line = rb_read(rb);
        if (line == NULL)
        {
            break; // 输入结束且缓冲区已空，退出
        }
        printf("Received: %s\n", line);
        free(line);
    }

    printf("Output thread finished.\n");
    return NULL;
}

int main()
{
    ring_buffer_t rb;
    pthread_t tid_in, tid_out;

    // 初始化环形缓冲区
    rb_init(&rb, BUFFER_SIZE);

    // 创建输入和输出线程
    if (pthread_create(&tid_in, NULL, input_thread, &rb) != 0)
    {
        perror("pthread_create input");
        rb_destroy(&rb);
        return 1;
    }
    if (pthread_create(&tid_out, NULL, output_thread, &rb) != 0)
    {
        perror("pthread_create output");
        rb_set_input_done(&rb); // 让输入线程也能退出
        pthread_join(tid_in, NULL);
        rb_destroy(&rb);
        return 1;
    }

    // 等待两个线程结束
    pthread_join(tid_in, NULL);
    pthread_join(tid_out, NULL);

    // 清理资源
    rb_destroy(&rb);

    return 0;
}