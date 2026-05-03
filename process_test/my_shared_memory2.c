#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    // 共享内存对象
    char shmName[100] = {0};
    sprintf(shmName, "/letter%d", getpid());
    // 1. 创建共享内存对象
    int fd = shm_open(shmName, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
    {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    // 2. 指定共享内存对象大小
    int shmSize = 100;
    if (ftruncate(fd, shmSize) != 0)
    {
        perror("ftruncate");
        close(fd);
        shm_unlink(shmName);
        exit(EXIT_FAILURE);
    }
    // 3. 内存映射
    char *shm = mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED)
    {
        perror("mmap 失败");
        close(fd);
        shm_unlink(shmName);
        exit(EXIT_FAILURE);
    }
    close(fd);

    // 4. 父子进程之间通过共享内存通信
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        munmap(shm, shmSize);
        shm_unlink(shmName); // ❗️解除映射，清理内存对象
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // 子进程
        sprintf(shm, "父进程%d你好，我是子进程%d", getppid(), getpid());
        printf("子进程写入共享内存完成\n");
    }
    else
    {
        // 父进程
        waitpid(pid, NULL, 0);
        printf("父进程读到：\"%s\"\n", shm);
        munmap(shm, shmSize); // ❗️解除映射，清理内存对象
        shm_unlink(shmName);
    }

    return 0;
}
