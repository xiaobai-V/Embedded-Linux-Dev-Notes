#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

/**
 * POSIX 共享内存示例：使用 shm_open 在父子进程间共享数据
 *
 * 与 shared_memory.c（mmap MAP_ANONYMOUS）的区别：
 *   MAP_ANONYMOUS：只能用于有亲缘关系的进程（fork 继承映射）
 *   shm_open：创建一个具名共享内存对象，任意进程都能通过名字找到它
 *             本质上是在 /dev/shm/ 下创建一个文件
 *
 * 三步走：shm_open 创建 → ftruncate 设置大小 → mmap 映射到地址空间
 */

int main()
{
    // 共享内存名字必须以 / 开头，且不包含其他 /
    char shmName[100] = {0};
    sprintf(shmName, "/letter%d", getpid());

    // ================================================================
    // 第1步：创建共享内存对象
    // ================================================================

    /**
     * @brief 创建或打开一个 POSIX 共享内存对象
     *
     * @def int shm_open(const char *name, int oflag, mode_t mode)
     *
     * @param name  共享内存对象名称，格式必须为 "/somename"
     *              实际创建在 /dev/shm/somename 文件
     * @param oflag 打开标志，与 open() 相同：
     *              O_CREAT - 不存在则创建
     *              O_RDWR  - 可读写
     *              O_EXCL  - 配合 O_CREAT，已存在则失败
     * @param mode  权限（创建时生效），如 0644
     *
     * @return 成功返回文件描述符，失败返回 -1 并设置 errno
     *
     * @note 返回的是文件描述符，但不是普通文件，不能直接 read/write
     *       必须配合 mmap 才能访问其中的数据
     *       编译时需要链接 rt 库：gcc -lrt
     */
    int fd = shm_open(shmName, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
    {
        perror("shm_open 失败");
        exit(EXIT_FAILURE);
    }

    // ================================================================
    // 第2步：设置共享内存大小
    // ================================================================

    /**
     * @brief 调整文件或共享内存对象的大小
     *
     * @def int ftruncate(int fd, off_t length)
     *
     * @param fd     shm_open 返回的文件描述符
     * @param length 目标大小（字节）
     *
     * @return 成功返回 0，失败返回 -1 并设置 errno
     *
     * @note 新创建的共享内存对象大小为 0，必须 ftruncate 设置大小后才能 mmap
     *       如果扩大，新增区域填充零；如果缩小，尾部数据丢失
     */
    size_t shm_size = 100;
    if (ftruncate(fd, shm_size) == -1)
    {
        perror("ftruncate 失败");
        close(fd);
        shm_unlink(shmName);
        exit(EXIT_FAILURE);
    }

    // ================================================================
    // 第3步：映射到进程地址空间
    // ================================================================

    /**
     * @brief 将共享内存对象映射到进程地址空间
     *
     * @note 与 shared_memory.c 中的 MAP_ANONYMOUS 不同：
     *       这里 fd 是 shm_open 返回的文件描述符（不是 -1）
     *       flag 没有 MAP_ANONYMOUS（映射的是具名共享内存对象）
     */
    char *share = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (share == MAP_FAILED)
    {
        perror("mmap 失败");
        close(fd);
        shm_unlink(shmName);
        exit(EXIT_FAILURE);
    }

    // mmap 完成后 fd 就不再需要了，关闭不影响已映射的内存
    close(fd);

    // ================================================================
    // fork：子进程写，父进程读
    // ================================================================

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        munmap(share, shm_size);
        shm_unlink(shmName);
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // 子进程写入共享内存
        sprintf(share, "你好父进程，我是子进程%d", getpid());
        printf("子进程%d 写入完成\n", getpid());
    }
    else
    {
        // 父进程等待子进程写入后读取
        waitpid(pid, NULL, 0);
        printf("父进程读到: \"%s\"\n", share);

        // 清理：解除映射 + 删除共享内存对象
        munmap(share, shm_size);

        /**
         * @brief 删除共享内存对象
         *
         * @def int shm_unlink(const char *name)
         *
         * @param name 共享内存对象名称（与 shm_open 一致）
         *
         * @return 成功返回 0，失败返回 -1 并设置 errno
         *
         * @note 类似文件的 unlink：删除名称引用，但已映射的进程仍可访问
         *       直到所有进程都 munmap 后物理内存才真正释放
         *       如果不调用 shm_unlink，/dev/shm/ 下的文件会一直存在
         */
        shm_unlink(shmName);
    }

    return 0;
}
