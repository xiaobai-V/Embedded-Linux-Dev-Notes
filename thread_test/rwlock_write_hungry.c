#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
static pthread_rwlock_t rwlock;

int shared_data = 0;

// 写饥饿问题
// 读操作阻塞1s, 只要有一个读线程获取到了读写锁，写线程就会被阻塞，但是其他读线程可以继续执行

/*
this is reader1, value is 0
this is reader2, value is 0
this is reader3, value is 0
this is reader4, value is 0
this is reader5, value is 0
this is reader6, value is 0
this is writer1, shared_data++ is 1
this is writer2, shared_data++ is 2
*/
/*
this is writer1, shared_data++ is 1
this is reader1, value is 1
this is reader2, value is 1
this is reader3, value is 1
this is reader4, value is 1
this is reader5, value is 1
this is reader6, value is 1
this is writer2, shared_data++ is 2
*/

void *lock_reader(void *argv)
{
    pthread_rwlock_rdlock(&rwlock);
    printf("this is %s, value is %d\n", (char *)argv, shared_data);

    // 加入sleep
    sleep(1);

    pthread_rwlock_unlock(&rwlock);
}

void *lock_writer(void *argv)
{
    pthread_rwlock_wrlock(&rwlock);
    int tmp = shared_data + 1; // 两个写线程读到的shared_data都是0
    // sleep(1);
    shared_data = tmp;
    printf("this is %s, shared_data++ is %d\n", (char *)argv, shared_data);
    pthread_rwlock_unlock(&rwlock);
}

int main(void)
{
    // 显示初始化读写锁
    pthread_rwlock_init(&rwlock, NULL);

    pthread_t writer1, writer2;
    pthread_t reader1, reader2, reader3, reader4, reader5, reader6;

    // 创建线程
    // 打乱顺序
    pthread_create(&writer1, NULL, lock_writer, "writer1");

    pthread_create(&reader1, NULL, lock_reader, "reader1");
    pthread_create(&reader2, NULL, lock_reader, "reader2");
    pthread_create(&reader3, NULL, lock_reader, "reader3");

    pthread_create(&writer2, NULL, lock_writer, "writer2");

    pthread_create(&reader4, NULL, lock_reader, "reader4");
    pthread_create(&reader5, NULL, lock_reader, "reader5");
    pthread_create(&reader6, NULL, lock_reader, "reader6");

    // 等待线程结束
    pthread_join(writer1, NULL);
    pthread_join(writer2, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);
    pthread_join(reader3, NULL);
    pthread_join(reader4, NULL);
    pthread_join(reader5, NULL);
    pthread_join(reader6, NULL);

    printf("shared_data: %d\n", shared_data);

    // 销毁读写锁
    pthread_rwlock_destroy(&rwlock);

    return 0;
}
