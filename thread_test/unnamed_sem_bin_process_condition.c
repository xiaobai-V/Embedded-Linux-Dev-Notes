#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    char *shm_value_name = "/unnamed_sem_shm_value";
    // 创建共享内存对象
    int value_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);
    // 调整共享内存对象的大小
    ftruncate(value_fd, sizeof(int));
    // 将共享内存对象映射到内存区域
    int *value = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, value_fd, 0);
    *value = 0;

    // 不再需要共享内存对象文件描述符
    close(value_fd);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
    }

    if (pid == 0)
    {
        // 子进程
        int tmp = *value + 1;
        sleep(1);
        *value = tmp;
    }
    else
    {
        // 父进程
        int tmp = *value + 1;
        sleep(1);
        *value = tmp;

        // 等待子进程执行完毕
        waitpid(pid, NULL, 0);
        printf("this is father, child finished\n");
        // 出现竞态条件
        printf("the final value is %d\n", *value); // the final value is 1
    }

    if (munmap(value, sizeof(int)) != 0)
    {
        perror("munmap");
    }

    if (pid > 0)
    {
        // 只在父进程中释放一次共享内存对象
        if (shm_unlink(shm_value_name) != 0)
        {
            perror("shm_unlink");
        }
    }

    return 0;
}
