#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

/**
 * 共享内存示例：使用 mmap 在父子进程间共享数据
 *
 * 与管道/FIFO 的区别：
 *   管道：数据在内核空间拷贝传递（write → 内核缓冲区 → read）
 *   共享内存：父子进程映射同一块物理内存，零拷贝，速度最快
 *
 * 本例使用 MAP_SHARED | MAP_ANONYMOUS：
 *   - MAP_ANONYMOUS：不依赖文件，内存由内核自动初始化为零
 *   - MAP_SHARED： fork 后子进程继承映射，父子共享同一块物理内存
 */

int main()
{
    // ================================================================
    // mmap 方式共享内存（匿名映射，无需文件）
    // ================================================================

    /**
     * @brief 将文件或设备映射到进程的地址空间（此处为匿名映射）
     *
     * @def void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
     *
     * @param addr    建议的映射起始地址，传 NULL 让内核自动选择
     * @param length  映射长度（字节数），会按页大小对齐
     * @param prot    内存保护标志：
     *                PROT_READ  - 可读
     *                PROT_WRITE - 可写
     *                PROT_EXEC  - 可执行
     *                PROT_NONE  - 不可访问
     * @param flags   映射类型标志：
     *                MAP_SHARED    - 共享映射，写入对其他进程可见
     *                MAP_PRIVATE  - 私有映射（写时复制）
     *                MAP_ANONYMOUS - 匿名映射，不关联任何文件，fd 传 -1
     * @param fd      文件描述符，匿名映射时传 -1
     * @param offset  文件偏移量，匿名映射时传 0
     *
     * @return 成功返回映射区域的指针，失败返回 MAP_FAILED（(void *)-1）
     *
     * @note MAP_SHARED | MAP_ANONYMOUS 组合是父子进程共享内存的最简方式，
     *       无需创建文件，无需 shm_open，fork 后子进程自动继承映射
     */
    size_t shm_size = 4096; // 通常取页大小的整数倍
    char *shm = mmap(NULL, shm_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shm == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // 初始状态下共享内存内容全为零（内核保证）
    printf("共享内存初始内容: \"%s\"\n", shm); // 应为空字符串

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        munmap(shm, shm_size);
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // ===== 子进程：向共享内存写入数据 =====
        sprintf(shm, "你好父进程，我是子进程%d", getpid());
        printf("子进程%d 写入完成\n", getpid());
        exit(EXIT_SUCCESS);
    }
    else
    {
        // ===== 父进程：等待子进程写入后读取 =====
        waitpid(pid, NULL, 0); // 等子进程写完再读（无同步机制时的简单方案）
        printf("父进程读到: \"%s\"\n", shm);

        /**
         * @brief 解除内存映射
         *
         * @def int munmap(void *addr, size_t length)
         *
         * @param addr   mmap 返回的映射地址
         * @param length 映射长度（与 mmap 时一致）
         *
         * @return 成功返回 0，失败返回 -1 并设置 errno
         *
         * @note 进程退出时内核会自动解除映射，但显式调用是好习惯
         *       MAP_ANONYMOUS 映射无需 shm_unlink（因为没有文件系统对象）
         */
        munmap(shm, shm_size);
    }

    return 0;
}
