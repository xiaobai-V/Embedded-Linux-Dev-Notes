#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    char *sem_name = "/named_sem";
    char *shm_name = "/named_sem_shm";
    // 初始化有名信号量
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
    // 初始化共享内存对象
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    // 分配共享内存大小
    ftruncate(fd, sizeof(int));
    // 将共享内存对象映射到内存空间
    int *value = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // 初始化共享内存对象的值
    *value = 0;
    // 关闭共享内存描述符
    close(fd);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
    }

    // 父子进程共同执行
    // 有名信号量用作互斥锁
    sem_wait(sem);
    int tmp = *value + 1;
    sleep(1);
    *value = tmp;
    sem_post(sem);
    // 父子进程都应该关闭对有名信号量的应用
    sem_close(sem);
    if (pid == 0)
    {
        // 子进程
    }
    else
    {
        // 父进程
        waitpid(pid, NULL, 0);
        printf("子进程结束， value = %d\n", *value);
        // 有名信号量取消链接 只执行一次
        sem_unlink(sem_name);
    }

    munmap(value, sizeof(int));

    if (pid > 0)
    {
        if (shm_unlink(shm_name) == -1)
        {
            perror("shm_unlink");
        }
    }

    return 0;
}