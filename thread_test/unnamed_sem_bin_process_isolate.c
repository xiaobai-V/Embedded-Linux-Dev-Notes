#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

int main()
{
    char *shm_value_name = "/unnamed_sem_shm_value";
    // 创建共享内存对象
    int value_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);
    // 调整共享内存对象的大小
    ftruncate(value_fd, sizeof(int));
    // 将共享内存对象映射到内存区域
    int *value = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, value_fd, 0);
    sem_t sem;
    // 初始化信号量和共享变量的值
    sem_init(&sem, 1, 1); // 第二个参数传入1代表用于`进程`间通信
    *value = 0;

    // 关闭共享内存对象文件描述符
    close(value_fd);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
    }

    if (pid == 0)
    {
        // 子进程
        sem_wait(&sem);

        int tmp = *value + 1;
        sleep(1);
        *value = tmp;

        sem_post(&sem);
    }
    else
    {
        // 父进程
        sem_wait(&sem);

        int tmp = *value + 1;
        sleep(1);
        *value = tmp;

        sem_post(&sem);

        // 等待子进程执行完毕
        waitpid(pid, NULL, 0);
        printf("this is father, child finished\n");
        printf("the final value is %d\n", *value);

        // 父进程销毁信号量
        sem_destroy(&sem);
    }

    // 解除共享变量共享内存的映射
    if (munmap(value, sizeof(int)) != 0)
    {
        perror("munmap value");
    }

    // 必须要先解除共享内存的映射(munmap)再删除共享内存(shm_unlink)
    if (pid > 0)
    {
        // 只在父进程中释放一次共享内存对象
        if (shm_unlink(shm_value_name) != 0)
        {
            perror("shm_unlink value failed");
        }
    }

    return 0;
}
