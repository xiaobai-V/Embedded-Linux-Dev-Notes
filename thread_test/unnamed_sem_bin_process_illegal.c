#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

// 一些底层的解释
/*
  Linux 无名信号量底层使用 futex，pshared 参数决定了 futex 的类型：

  ┌─────────┬───────────────┬───────────────────────────────────────┐
  │ pshared │  futex 类型   │                 特性                  │
  ├─────────┼───────────────┼───────────────────────────────────────┤
  │ 0       │ FUTEX_PRIVATE │ 按进程 mm_struct 哈希，跨进程唤醒无效 │
  ├─────────┼───────────────┼───────────────────────────────────────┤
  │ 1       │ FUTEX_SHARED  │ 按物理地址哈希，跨进程唤醒有效        │
  └─────────┴───────────────┴───────────────────────────────────────┘

  无 sleep 时——碰巧不阻塞

  没有 sleep 时，临界区极短，执行顺序几乎总是串行的：

  进程A: sem_wait → 原子减到0 → tmp+1 → 赋值 → sem_post → 原子恢复为1
  进程B:                                          sem_wait → 原子减到0 → ...

  两个进程都走了 futex 快速路径（原子操作直接成功，无需进入内核等待），PRIVATE 还是 SHARED 根本没区别，所以 pshared=0 也能正常工作。

  有 sleep 时——死锁

  // 父进程                          // 子进程
  sem_wait(sem);        // 值→0
  sleep(1);             // 休眠       sem_wait(sem);  // 值已经是0
                        //           → futex_wait(PRIVATE) 阻塞
                        //           等待父进程唤醒
  *value = tmp;
  sem_post(sem);        // 值→1
                        // futex_wake(PRIVATE)
                        // 只在父进程的 mm_struct 中查找
                        // 找不到子进程！子进程永远不会被唤醒
  waitpid(pid, ...);    // 永远等不到子进程结束 → 死锁

  子进程在 futex_wait(PRIVATE) 上永远阻塞，父进程在 waitpid 上永远等待 → 死锁。

  总结

  ┌──────────┬──────────────────────────────────────┬───────────┐
  │   场景   │              pshared=0               │ pshared=1 │
  ├──────────┼──────────────────────────────────────┼───────────┤
  │ 无 sleep │ 正常（快速路径，未阻塞）             │ 正常      │
  ├──────────┼──────────────────────────────────────┼───────────┤
  │ 有 sleep │ 死锁（PRIVATE futex 跨进程唤醒失效） │ 正常      │
  └──────────┴──────────────────────────────────────┴───────────┘

  所以文件名中的 illegal 是准确的——pshared=0 用于进程间同步确实是不合法的，只是在不触发阻塞路径时表现出了"正常"的假象。sleep
  放大了竞态窗口，让 bug 暴露出来。
*/

int main()
{
    char *shm_sem_name = "/unnamed_sem_shm_sem";
    char *shm_value_name = "/unnamed_sem_shm_value";
    // 创建共享内存对象
    int sem_fd = shm_open(shm_sem_name, O_CREAT | O_RDWR, 0666);
    int value_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);
    // 调整共享内存对象的大小
    ftruncate(sem_fd, sizeof(sem_t));
    ftruncate(value_fd, sizeof(int));
    // 将共享内存对象映射到内存区域
    sem_t *sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, sem_fd, 0);
    int *value = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, value_fd, 0);
    // 初始化信号量和共享变量的值
    // 传入0关闭信号量进程间共享
    sem_init(sem, 0, 1);
    *value = 0;
    // 不再需要共享内存对象文件描述符
    close(sem_fd);
    close(value_fd);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
    }

    if (pid == 0)
    {
        // 子进程
        sem_wait(sem);

        int tmp = *value + 1;
        // sleep(1);
        *value = tmp;

        sem_post(sem);
    }
    else
    {
        // 父进程
        sem_wait(sem);

        int tmp = *value + 1;
        // sleep(1);
        *value = tmp;

        sem_post(sem);

        // 等待子进程执行完毕
        waitpid(pid, NULL, 0);
        printf("this is father, child finished\n");
        // 使用信号量解决竞态条件后，输出始终为2
        printf("the final value is %d\n", *value);
    }

    // 解除信号量共享内存的映射
    if (munmap(sem, sizeof(sem_t)) != 0)
    {
        perror("munmap sem");
    }

    // 解除共享变量共享内存的映射
    if (munmap(value, sizeof(int)) != 0)
    {
        perror("munmap value");
    }

    if (pid > 0)
    {
        // 只在父进程中释放一次共享内存对象
        if (shm_unlink(shm_value_name) != 0)
        {
            perror("shm_unlink value failed");
        }
        if (shm_unlink(shm_sem_name) != 0)
        {
            perror("shm_unlink sem failed");
        }
    }

    return 0;
}
