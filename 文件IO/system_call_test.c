#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // read, write, close, lseek, dup, dup2, access
#include <fcntl.h>    // open, oflag 常量
#include <sys/stat.h> // open 的 mode 权限参数 (O_CREAT 时需要)
#include <string.h>   // strlen
#include <errno.h>    // errno

int main(int argc, char const *argv[])
{
    // ========================================================
    // 第一部分：open/read/write/close 基本文件操作
    // ========================================================

    // open打开文件
    /**
     * @brief 简述：open系统调用打开文件
     *
     * @def 函数原型：int open (const char *__path, int __oflag, ...)
     *
     * @param const char *__path：
     *      含义：文件路径
     *
     * @param int __oflag：
     *      含义：打开文件的模式, man 2 open查看
     *      （1）：O_RDONLY: 只读模式
     *      （2）：O_WRONLY: 只写模式
     *      （3）：O_RDWR:   读写模式
     *      （4）：O_CREAT:  文件不存在时创建，需提供第三个参数指定权限
     *      （5）：O_TRUNC:  截断文件为0长度（需写权限）
     *      （6）：O_APPEND: 追加模式，每次写入前偏移量移到末尾
     *
     * @param ...：
     *      含义：只有模式含有 O_CREAT 且文件不存在的时候，可以用于指定新创建文件的权限位
     *
     * @return 返回值：
     *      成功：返回非负的文件描述符
     *      失败：返回-1，设置errno指示错误原因
     *
     * @note 注意事项/易错点：
     *      O_RDONLY / O_WRONLY / O_RDWR 是互斥的，只能选一个
     *      O_CREAT 需要第三个参数指定权限（如 0644），否则权限为随机值
     */
    int fd = open("io.txt", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE); // 依赖stdlib.h
    }

    // read读取文件
    char buffer[1024];
    ssize_t bytes_read;

    /**
     * @brief 简述：read系统调用读取文件
     *
     * @def 函数原型：ssize_t read (int __fd, void *__buf, size_t __nbytes)
     *
     * @param int __fd：
     *      含义：文件描述符
     *
     * @param void *__buf：
     *      含义：要写入的缓冲区的指针
     *
     * @param size_t __nbytes：
     *      含义：读取的最大字节数，实际可能会少于这个数量
     *
     * @return 返回值：
     *      成功：返回实际读取的字节数，可能小于__nbytes
     *      到达文件末尾：返回0
     *      失败：返回-1，设置errno
     *
     * @note 注意事项/易错点：
     *      read 返回 0 表示 EOF（文件末尾），不是错误
     *      read 可能被信号中断，此时 errno == EINTR，应重新调用
     */
    printf("=== open/read/write/close 基本示例 ===\n");
    fflush(stdout); // 刷新 printf 的缓冲区，避免与 write(STDOUT_FILENO) 混用时顺序错乱
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        /**
         * @brief 简述：write系统调用写出数据
         *
         * @def 函数原型：ssize_t write (int __fd, const void *__buf, size_t __n)
         *
         * @param int __fd：
         *      含义：文件描述符
         *
         * @param const void *__buf：
         *      含义：要写出的数据缓冲区指针
         *
         * @param size_t __n：
         *      含义：要写出的字节数
         *
         * @return 返回值：
         *      成功：返回实际写出的字节数，可能小于__n（部分写）
         *      失败：返回-1，设置errno
         *
         * @note 注意事项/易错点：
         *      write 可能只写出部分数据（部分写），需要循环写出确保全部写入
         */

        /* Standard file descriptors.  */
        // #define STDIN_FILENO  0  /* Standard input.  */
        // #define STDOUT_FILENO 1  /* Standard output.  */
        // #define STDERR_FILENO 2  /* Standard error output.  */
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    if (bytes_read == -1)
    {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    /**
     * @brief 简述：close系统调用关闭文件描述符
     *
     * @def 函数原型：int close (int __fd)
     *
     * @param int __fd：
     *      含义：要关闭的文件描述符
     *
     * @return 返回值：
     *      成功：返回0
     *      失败：返回-1，设置errno
     *
     * @note 注意事项/易错点：
     *      关闭后不应再使用该文件描述符
     *      进程终止时内核会自动关闭所有打开的文件描述符
     */
    close(fd); // 使用完毕后关闭文件描述符

    // ========================================================
    // 第二部分：open O_CREAT 模式 — 创建文件并写入
    // ========================================================
    printf("\n=== open O_CREAT 示例 ===\n");

    /**
     * @brief 简述：open 以 O_CREAT 模式创建新文件并写入
     *
     * O_CREAT | O_WRONLY | O_TRUNC 组合含义：
     *   - O_CREAT: 文件不存在则创建
     *   - O_WRONLY: 只写模式打开
     *   - O_TRUNC: 如果文件已存在，截断为0长度
     * 第三个参数 0644 = 所有者读写(6) + 组用户只读(4) + 其他用户只读(4)
     */
    int fd_write = open("system_call_output.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd_write == -1)
    {
        perror("open O_CREAT");
        exit(EXIT_FAILURE);
    }

    char *write_msg = "This data is written by write() system call.\n";
    ssize_t bytes_written = write(fd_write, write_msg, strlen(write_msg));
    if (bytes_written == -1)
    {
        perror("write to file");
        close(fd_write);
        exit(EXIT_FAILURE);
    }
    printf("Written %zd bytes to system_call_output.txt\n", bytes_written);
    close(fd_write);

    // ========================================================
    // 第三部分：lseek — 调整文件偏移量
    // ========================================================
    printf("\n=== lseek 示例 ===\n");

    /**
     * @brief 简述：lseek系统调用改变文件的读写偏移量
     *
     * @def 函数原型：off_t lseek (int __fd, off_t __offset, int __whence)
     *
     * @param int __fd：
     *      含义：文件描述符
     *
     * @param off_t __offset：
     *      含义：偏移量（正负均可，单位字节）
     *
     * @param int __whence：
     *      含义：偏移基准位置
     *      SEEK_SET: 文件开头（offset 非负）
     *      SEEK_CUR: 当前位置
     *      SEEK_END: 文件末尾
     *
     * @return 返回值：
     *      成功：返回新的文件偏移量（从文件开头计算的字节数）
     *      失败：返回(off_t)-1，设置errno
     *
     * @note 注意事项/易错点：
     *      lseek 可以越过文件末尾设置偏移量，后续写入会形成"文件空洞"（hole）
     *      lseek 不适用于管道、套接字、FIFO（会返回 ESPIPE 错误）
     *      lseek 只是修改内核中的偏移量，不会引起实际的 I/O 操作
     */
    int fd_lseek = open("system_call_output.txt", O_RDONLY);
    if (fd_lseek == -1)
    {
        perror("open for lseek");
        exit(EXIT_FAILURE);
    }

    // 先读取全部内容
    char lseek_buf[256];
    ssize_t n;
    printf("--- lseek: read full file ---\n");
    while ((n = read(fd_lseek, lseek_buf, sizeof(lseek_buf))) > 0)
    {
        write(STDOUT_FILENO, lseek_buf, n);
    }

    // 使用 lseek 回到文件开头，再读取前 10 字节
    off_t pos = lseek(fd_lseek, 0, SEEK_SET);
    if (pos == (off_t)-1)
    {
        perror("lseek");
        close(fd_lseek);
        exit(EXIT_FAILURE);
    }
    printf("--- lseek to beginning, read 10 bytes ---\n");
    n = read(fd_lseek, lseek_buf, 10);
    if (n > 0)
    {
        write(STDOUT_FILENO, lseek_buf, n);
        printf("\n");
    }
    close(fd_lseek);

    // ========================================================
    // 第四部分：dup / dup2 — 复制文件描述符
    // ========================================================
    printf("\n=== dup/dup2 示例 ===\n");

    /**
     * @brief 简述：dup/dup2 复制文件描述符
     *
     * @def int dup (int __fd)
     * @def int dup2 (int __fd, int __fd2)
     *
     * @param int __fd：
     *      含义：要复制的源文件描述符
     *
     * @param int __fd2 (仅dup2)：
     *      含义：目标文件描述符，如果已打开则先关闭再复制
     *
     * @return 返回值：
     *      dup:  成功返回新的（最小可用）文件描述符，失败返回-1
     *      dup2: 成功返回 __fd2，失败返回-1
     *
     * @note 注意事项/易错点：
     *      dup 返回最小可用的文件描述符（参考文件描述符分配规则）
     *      dup2 是原子操作（close + dup），不会出现 close 和 dup 之间的竞争
     *      复制后的 fd 和原 fd 共享同一个文件表项（共享偏移量）
     *      各自 close 互不影响，需要各自关闭
     */
    int fd_dup_src = open("system_call_output.txt", O_RDONLY);
    if (fd_dup_src == -1)
    {
        perror("open for dup");
        exit(EXIT_FAILURE);
    }

    // dup: 复制文件描述符，返回最小可用的 fd
    int fd_dup_new = dup(fd_dup_src);
    if (fd_dup_new == -1)
    {
        perror("dup");
        close(fd_dup_src);
        exit(EXIT_FAILURE);
    }
    printf("original fd = %d, dup fd = %d\n", fd_dup_src, fd_dup_new);

    // 通过复制的 fd 读取内容（验证共享偏移量）
    char dup_buf[64];
    ssize_t dup_n = read(fd_dup_new, dup_buf, sizeof(dup_buf) - 1);
    if (dup_n > 0)
    {
        dup_buf[dup_n] = '\0';
        printf("Read from dup fd: %s", dup_buf);
    }

    close(fd_dup_src);
    close(fd_dup_new);

    // ========================================================
    // 第五部分：access — 检查文件访问权限
    // ========================================================
    printf("\n=== access 示例 ===\n");

    /**
     * @brief 简述：access系统调用检查调用进程对文件的访问权限
     *
     * @def 函数原型：int access (const char *__name, int __type)
     *
     * @param const char *__name：
     *      含义：文件路径
     *
     * @param int __type：
     *      含义：要检查的权限
     *      F_OK: 文件是否存在
     *      R_OK: 可读
     *      W_OK: 可写
     *      X_OK: 可执行
     *
     * @return 返回值：
     *      成功（具有权限）：返回0
     *      失败（无权限或文件不存在）：返回-1，设置errno
     *
     * @note 注意事项/易错点：
     *      access 检查的是实际用户ID（real UID）的权限，不是有效用户ID（effective UID）
     *      存在 TOCTOU（Time of Check to Time of Use）竞争风险：
     *          access 检查通过后到实际操作之间，文件可能被替换
     *      安全编程中不建议用 access + open，应直接 open 并检查错误
     */
    if (access("system_call_output.txt", F_OK) == 0)
    {
        printf("system_call_output.txt exists\n");
    }
    if (access("system_call_output.txt", R_OK) == 0)
    {
        printf("system_call_output.txt is readable\n");
    }
    if (access("system_call_output.txt", W_OK) == 0)
    {
        printf("system_call_output.txt is writable\n");
    }
    if (access("nonexistent.txt", F_OK) == -1)
    {
        printf("nonexistent.txt does not exist (errno = %d)\n", errno);
    }

    return 0;
}
