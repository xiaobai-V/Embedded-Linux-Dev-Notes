# Linux 进程知识总结

---

## 第一章：Linux 进程简介

### 1.1 什么是进程

**程序 vs 进程：**

| | 程序 | 进程 |
|---|---|---|
| 本质 | 磁盘上的可执行文件（静态） | 运行中的程序实例（动态） |
| 生命周期 | 永久存在 | 随创建而生，随退出而灭 |
| 资源 | 不占用 CPU/内存 | 拥有独立的地址空间、文件描述符表、PID |

**PCB（进程控制块）：** 内核为每个进程维护的数据结构（Linux 中为 `task_struct`），记录 PID、状态、优先级、文件描述符表、信号处理等所有信息。

### 1.2 进程状态

```
                    fork()
                      │
                      ▼
                ┌──────────┐
                │  就绪态   │ ← 等待 CPU 调度
                └────┬─────┘
                     │ 获得CPU
                     ▼
               ┌───────────┐   等待事件    ┌──────────┐
               │  运行态    │ ───────────→ │  阻塞态   │
               └─────┬─────┘               └────┬─────┘
                     │                          │ 事件完成
                     │ 调度器抢占                │
                     ▼                          ▼
               ┌───────────┐              ┌──────────┐
               │  就绪态    │ ←───────────│  就绪态   │
               └───────────┘              └──────────┘
                     │
                     │ exit()
                     ▼
               ┌───────────┐   waitpid()   ┌──────────┐
               │  僵尸态    │ ───────────→ │   销毁    │
               └───────────┘               └──────────┘
```

| 状态 | 说明 |
|---|---|
| **就绪（Ready）** | 进程已准备好运行，等待 CPU 调度 |
| **运行（Running）** | 正在 CPU 上执行 |
| **阻塞（Blocked/Sleeping）** | 等待 I/O、信号、锁等事件，不占用 CPU |
| **僵尸（Zombie）** | 进程已退出，但父进程尚未调用 `waitpid` 回收，PCB 仍保留 |
| **孤儿（Orphan）** | 父进程先于子进程退出，子进程被 init(PID=1) 领养 |

> **僵尸进程的危害：** 僵尸进程虽然不占用 CPU 和内存，但内核保留其 PCB。大量僵尸会耗尽 PID 资源。解决方法：父进程调用 `waitpid` 回收。

### 1.3 进程树

Linux 所有进程按父子关系组成树，根节点是 PID=1 的 init 进程（现代系统为 systemd）。

```
systemd (PID=1)
├── sshd (PID=100)
│   └── bash (PID=200)
│       └── vim (PID=300)
├── cron (PID=101)
└── nginx (PID=102)
    ├── worker (PID=103)
    └── worker (PID=104)
```

**常用命令：**

```bash
pstree -p         # 查看进程树（显示PID）
ps -ef            # 查看所有进程
ps -o pid,ppid,cmd  # 查看PID/PPID/命令
```

### 1.4 用户空间与内核空间

```
┌─────────────────────────────────┐
│          用户空间                 │  应用程序运行区域
│  （你的代码、printf、fopen...）    │  权限受限，不能直接操作硬件
├─────────────────────────────────┤
│         ↓ 系统调用 ↑             │  唯一的跨界通道
├─────────────────────────────────┤
│          内核空间                 │  内核运行区域
│  （进程调度、内存管理、驱动...）    │  完全权限，操作所有硬件
└─────────────────────────────────┘
```

**系统调用（syscall）** 是用户程序请求内核服务的唯一接口。本章后续所有 API（fork、read、write 等）本质上都是系统调用。

---

## 第二章：进程相关系统调用

### 2.1 fork() — 创建子进程

```c
#include <unistd.h>
pid_t fork(void);
```

**核心行为：** 创建一个与父进程几乎完全相同的子进程。子进程复制父进程的代码段、数据段、堆、栈、文件描述符表。

**返回值：**

| 返回值 | 含义 |
|---|---|
| `> 0` | 父进程中，返回子进程的 PID |
| `== 0` | 子进程中 |
| `-1` | 失败 |

**典型用法：** 通过返回值区分父子进程的执行路径。

```c
pid_t pid = fork();
if (pid == -1) { perror("fork"); }
else if (pid == 0) { /* 子进程逻辑 */ }
else { /* 父进程逻辑 */ }
```

**文件描述符继承：** fork 后子进程继承父进程的文件描述符表，父子进程共享同一个文件表项（共享文件偏移量）。使用 `O_APPEND` 可保证原子写入。

> **示例：** `fork_test.c`（基本 fork）、`fork_fd_test.c`（文件描述符继承 + O_APPEND）、`process_test.c`（fork 后父子地址空间独立）

**关键理解：** fork 后父子进程的变量各自独立（写时复制），修改互不影响：

```
fork() 之前:  num = 0（父进程）

fork() 之后:
  父进程:  num++ → num=1（只影响自己的副本）
  子进程:  num++ → num=1（只影响自己的副本）
```

### 2.2 execve() — 替换进程映像

```c
#include <unistd.h>
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

**核心行为：** 用新程序替换当前进程的代码段、数据段、堆和栈。PID 不变，但程序完全变成了新程序。

| 参数 | 含义 |
|---|---|
| `pathname` | 要执行的程序路径 |
| `argv[]` | 参数数组，`argv[0]` 为程序名，末尾必须 `NULL` |
| `envp[]` | 环境变量数组，格式 `key=value`，末尾必须 `NULL` |

**返回值：** 成功时不返回（进程映像已被替换），失败返回 -1。

**fork + execve 组合：** 这是 Linux 创建新进程的标准模式——fork 创建子进程，execve 让子进程执行新程序。

```
父进程                    子进程
  │                         │
  ├─ fork() ────────────────┤
  │                         │
  │  继续执行父进程逻辑       │  execve("./erlou", ...)
  │                         │  ↓ 进程映像被替换
  │                         │  变成 erlou 程序
  │                         │
  ├─ waitpid() ←────────────┤ 子进程退出
```

> **示例：** `execve_test.c`（基本 execve）、`fork_execve_test.c`（fork+execve 组合）、`pstree_test.c`（观察进程树）

**exec 家族：** execve 是系统调用，其他（execl、execlp、execv、execvp、execvpe）都是库函数，最终调用 execve。

### 2.3 waitpid() — 等待子进程

```c
#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *status, int options);
```

| 参数 | 含义 |
|---|---|
| `pid` | 指定子进程 PID；`-1` 表示等待任意子进程 |
| `status` | 子进程退出状态（可用 `WIFEXITED`、`WEXITSTATUS` 等宏解析） |
| `options` | `0` 阻塞等待；`WNOHANG` 非阻塞（立即返回） |

**返回值：** 成功返回子进程 PID；`WNOHANG` 且子进程未退出时返回 0；失败返回 -1。

**为什么要 wait：**
- 子进程退出后，内核保留其 PCB（变成僵尸态）
- 父进程必须调用 `wait/waitpid` 回收 PCB，否则僵尸进程累积

> **示例：** `waitpid_test.c`

### 2.4 exit() / _exit() — 进程终止

```c
#include <stdlib.h>
void exit(int status);      // 标准库函数：会刷新 stdio 缓冲区，调用 atexit 注册的函数

#include <unistd.h>
void _exit(int status);     // 系统调用：直接退出，不清理缓冲区
```

| 场景 | 选择 |
|---|---|
| 父进程退出 | `exit()` — 需要清理缓冲区 |
| 子进程退出（fork 后） | `_exit()` — 避免重复刷新父进程的缓冲区 |

### 2.5 system() — 执行 Shell 命令

```c
#include <stdlib.h>
int system(const char *command);
```

**本质：** 封装了 `fork + execve + waitpid`。内部启动 `/bin/sh -c "command"` 执行命令。

**返回值：** 命令的退出状态码。

> **示例：** `system_test.c`

### 2.6 getpid() / getppid() — 获取进程 ID

```c
#include <unistd.h>
pid_t getpid(void);    // 获取当前进程 PID
pid_t getppid(void);   // 获取父进程 PID
```

### 2.7 错误处理 — errno 与 perror

系统调用失败时，内核设置全局变量 `errno` 指示错误原因。

```c
#include <errno.h>     // errno 定义
#include <stdio.h>     // perror 定义

extern int errno;
void perror(const char *s);  // 输出格式: "s: 错误描述\n"
```

**使用模式：**

```c
if (系统调用 == -1) {
    perror("操作描述");    // 例: "fork: Resource temporarily unavailable"
    // 或手动处理:
    // if (errno == EEXIST) { ... }
}
```

> **示例：** `perror_test.c`、`errno_test.c`

### 第二章示例索引

| 文件 | 演示内容 | 运行命令 |
|---|---|---|
| `fork_test.c` | fork 基本用法 | `make fork_test` |
| `fork_fd_test.c` | 文件描述符继承 + O_APPEND | `make fork_fd_test` |
| `process_test.c` | fork 后地址空间独立 | `make process_test` |
| `execve_test.c` | execve 程序替换 | `make execve_test` |
| `fork_execve_test.c` | fork + execve 组合 | `make fork_execve_test` |
| `pstree_test.c` | 进程树观察 | `make pstree_test` |
| `waitpid_test.c` | waitpid 等待子进程 | `make waitpid_test` |
| `orphan_process_test.c` | 孤儿进程 | `make orphan_process_test` |
| `system_test.c` | system 执行 shell 命令 | `make system_test` |
| `perror_test.c` | perror 错误输出 | `make perror_test` |
| `errno_test.c` | errno 全局变量 | `make errno_test` |

---

## 第三章：进程间通信（IPC）

### 为什么需要 IPC

每个进程有独立的地址空间，进程 A 的变量进程 B 无法直接访问。IPC 机制就是内核提供的"通信桥梁"。

### 3.1 匿名管道（pipe）

```c
#include <unistd.h>
int pipe(int pipefd[2]);
```

| 参数 | 含义 |
|---|---|
| `pipefd[0]` | 读端 |
| `pipefd[1]` | 写端 |

**返回值：** 成功返回 0，失败返回 -1。

**特点：**

| 属性 | 说明 |
|---|---|
| 半双工 | 数据只能单向流动（一端写，另一端读） |
| 亲缘限制 | 只能用于父子进程（fork 继承 fd） |
| 字节流 | 无消息边界，读端无法区分写入次数 |
| 内核缓冲 | 数据经过内核缓冲区拷贝（非零拷贝） |
| 生命周期 | 随进程，所有 fd 关闭后管道消失 |

**fd 引用计数与关闭时序（关键）：**

fork 后管道的每个端口在父子进程中各有一份拷贝。内核维护每个端口的引用计数，**只有引用计数归零时才生效**：

```
pipe() 后：读端引用=1（父），写端引用=1（父）
fork 后：读端引用=2（父+子），写端引用=2（父+子）

原则：谁不用的端口，谁就尽早关！
- 读进程关闭写端 → 读端的 read() 才能收到 EOF
- 写进程关闭读端 → 避免浪费
```

**数据流：**

```
写进程                          读进程
  │                               │
  │  write(pipefd[1], data)       │
  │ ──────→ 内核缓冲区 ──────→    │  read(pipefd[0], buf)
  │        (管道，内核空间)        │
  │                               │
  close(pipefd[1])                │
  (写端引用计数归零)               │  read 返回 0 → EOF
```

> **示例：** `unnamed_pipe_test.c`（父子通信）、`my_unnamed_pipe_test.c`（兄弟进程通信）

### 3.2 有名管道（FIFO）

```c
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode);
```

| 参数 | 含义 |
|---|---|
| `pathname` | 管道文件路径（创建在文件系统中） |
| `mode` | 权限（如 0644） |

**特点：**

| 属性 | 说明 |
|---|---|
| 无亲缘限制 | 任意进程通过文件路径找到对方 |
| 文件系统可见 | `ls -l` 显示为 `p` 类型（pipe） |
| 阻塞 open | `open(O_WRONLY)` 阻塞直到有进程 `open(O_RDONLY)`，反之亦然 |
| 用完需清理 | `unlink()` 删除管道文件 |

**FIFO 的阻塞 open（与普通文件的核心区别）：**

```
终端1: ./fifo_write                    终端2: ./fifo_read
  │                                      │
  │  mkfifo("/tmp/myfifo")               │
  │  open(O_WRONLY) ←── 阻塞 ──→        │  open(O_RDONLY)
  │              两端同时解除阻塞          │
  │  write(fd, data)  ──────────→        │  read(fd, buf)
  │  close(fd)                            │  read 返回 0 (EOF)
  │  unlink("/tmp/myfifo")               │  close(fd)
```

> **示例：** `fifo_write.c`、`fifo_read.c`

### 3.3 共享内存（mmap / shm_open）

**最快的 IPC：** 数据不需要在内核空间拷贝，多个进程直接访问同一块物理内存。

#### 方式一：mmap 匿名映射（仅父子进程）

```c
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
```

```c
// 创建匿名共享内存
char *shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

// fork 后子进程自动继承映射，父子共享同一块物理内存
```

#### 方式二：POSIX 共享内存（任意进程）

三步走：`shm_open` → `ftruncate` → `mmap`

```c
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int shm_open(const char *name, int oflag, mode_t mode);  // 创建共享内存对象
int ftruncate(int fd, off_t length);                      // 设置大小（新创建时大小为0）
int shm_unlink(const char *name);                         // 删除共享内存对象
```

```c
// 创建
int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0644);
ftruncate(fd, 4096);
char *shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
close(fd);  // mmap 后 fd 不再需要

// 删除
munmap(shm, 4096);
shm_unlink("/my_shm");
```

**两种方式对比：**

| | mmap MAP_ANONYMOUS | shm_open |
|---|---|---|
| 适用范围 | 仅父子进程 | 任意进程 |
| 创建步骤 | 一步：mmap | 三步：shm_open → ftruncate → mmap |
| 清理 | `munmap` | `munmap` + `shm_unlink` |
| 编译 | `gcc` | `gcc -lrt` |

> **示例：** `shared_memory.c`（匿名映射）、`shared_memory2.c`（POSIX 共享内存）

### 3.4 消息队列（POSIX mq）

**与管道的区别：** 消息队列传递的是有边界的消息（每条消息独立），而管道是字节流（无边界）。

```c
#include <mqueue.h>

mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                 unsigned int msg_prio, const struct timespec *abs_timeout);
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                        unsigned int *msg_prio, const struct timespec *abs_timeout);
int mq_close(mqd_t mqdes);
int mq_unlink(const char *name);
```

**mq_attr 属性结构体：**

```c
struct mq_attr {
    long mq_flags;    // 0（阻塞）或 O_NONBLOCK
    long mq_maxmsg;   // 队列最大消息数
    long mq_msgsize;  // 每条消息最大字节数
    long mq_curmsgs;  // 当前队列中的消息数（mq_open 时忽略）
};
```

**关键注意事项：**
- 消息队列名字必须以 `/` 开头（如 `/p_c_mq`）
- `mq_timedsend` 的 `msg_len` 应为实际数据长度，不是缓冲区大小
- `mq_timedreceive` 返回实际接收的字节数；返回 0 表示收到 0 字节消息
- 编译需要 `-lrt` 链接实时库
- `mq_unlink` 删除队列名称，但已打开的进程仍可继续使用

> **示例：** `producer.c`（生产者）、`consumer.c`（消费者）、`father_son_mq_test.c`（父子进程消息队列）

### 3.5 IPC 方式对比

| 方式 | 适用进程 | 数据拷贝 | 消息边界 | 同步机制 | 速度 |
|---|---|---|---|---|---|
| **匿名管道** | 父子 | 有（内核缓冲） | 无（字节流） | read 阻塞等待数据 | 中 |
| **有名管道** | 任意 | 有（内核缓冲） | 无（字节流） | open 阻塞等待对端 | 中 |
| **共享内存** | 任意 | 无（零拷贝） | 无 | 需自行同步（信号量） | 最快 |
| **消息队列** | 任意 | 有 | 有（按消息） | 超时/阻塞 | 中 |

**选型指南：**
- 父子进程简单通信 → 匿名管道
- 不相关进程简单通信 → 有名管道
- 大数据量高速传输 → 共享内存
- 需要结构化消息 + 优先级 → 消息队列

---

## 第四章：信号

### 4.1 信号概念

信号是内核向进程发送的**异步通知**。进程收到信号后，会中断当前执行流程，转而执行信号处理函数（或执行默认行为）。

**信号的生命周期：**

```
产生信号                    递达信号                    处理信号
（内核/进程/硬件）    →     （等待递达）      →     （执行处理动作）

产生方式：
  - 用户按键：Ctrl+C → SIGINT
  - 硬件异常：非法内存访问 → SIGSEGV
  - kill 命令：kill -9 PID → SIGKILL
  - 软件条件：管道读端关闭后写端写 → SIGPIPE
```

### 4.2 常见信号

| 信号 | 编号 | 默认行为 | 说明 |
|---|---|---|---|
| `SIGINT` | 2 | 终止进程 | Ctrl+C 产生 |
| `SIGTERM` | 15 | 终止进程 | kill 默认发送（可被捕获，用于优雅退出） |
| `SIGKILL` | 9 | 终止进程 | **不能被捕获、阻塞或忽略**（强制杀死） |
| `SIGSEGV` | 11 | 终止+core | 非法内存访问（段错误） |
| `SIGPIPE` | 13 | 终止进程 | 向已关闭的管道/Socket 写数据 |
| `SIGCHLD` | 17 | 忽略 | 子进程状态变化（退出/暂停/继续） |
| `SIGSTOP` | 19 | 暂停进程 | **不能被捕获或忽略**（Ctrl+Z 产生 SIGTSTP） |
| `SIGUSR1` | 10 | 终止进程 | 用户自定义信号1 |
| `SIGUSR2` | 12 | 终止进程 | 用户自定义信号2 |

> `SIGKILL`（9）和 `SIGSTOP`（19）是仅有的两个不能被捕获的信号。这就是为什么 `kill -9` 能杀死任何进程。

### 4.3 signal() — 注册信号处理函数

```c
#include <signal.h>
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
```

| 参数 | 含义 |
|---|---|
| `signum` | 要捕获的信号编号（如 `SIGINT`） |
| `handler` | 处理函数指针，或 `SIG_IGN`（忽略）、`SIG_DFL`（默认行为） |

**返回值：** 成功返回之前的处理函数指针，失败返回 `SIG_ERR`。

**使用示例：**

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sigint_handler(int sig) {
    printf("收到信号 %d (SIGINT)，但我不退出\n", sig);
}

int main() {
    signal(SIGINT, sigint_handler);  // 捕获 Ctrl+C

    while (1) {
        printf("运行中... PID=%d\n", getpid());
        sleep(1);
    }
    return 0;
}
```

> **示例：** `signal_test.c`

### 4.4 信号处理流程

```
用户进程正在执行代码
         │
         │  ← 内核递达信号（如 SIGINT）
         │
    ┌────┴────┐
    │ 保存当前 │
    │ 执行上下 │
    │ 文到栈中 │
    └────┬────┘
         │
         ▼
  ┌──────────────┐
  │  信号处理函数  │  ← 用户注册的 handler
  │  执行完毕      │
  └──────┬───────┘
         │
         ▼
    ┌────────────┐
    │ 恢复之前   │
    │ 保存的上下文 │
    └────┬───────┘
         │
         ▼
  进程从被中断处继续执行
```

### 4.5 信号注意事项

1. **不能捕获 SIGKILL 和 SIGSTOP**：这两个信号始终执行默认行为，这是内核的安全保障
2. **信号处理函数中只能调用异步信号安全函数**：如 `write`、`_exit` 等。**不能**调用 `printf`、`malloc`、`free` 等非安全函数（本仓库示例为学习目的使用了 `printf`，实际生产代码应避免）
3. **信号可能丢失**：标准信号（非实时信号）是 pending 位图，同一信号多次发送只计一次
4. **SIGCHLD 的作用**：子进程退出时内核向父进程发送 SIGCHLD，父进程可在处理函数中调用 `waitpid` 回收

### 第四章示例索引

| 文件 | 演示内容 | 运行命令 |
|---|---|---|
| `signal_test.c` | signal() 捕获 SIGINT | `make signal_test` |

---

## 全部示例程序索引

| 文件 | 章节关联 | 演示内容 | 运行命令 |
|---|---|---|---|
| `fork_test.c` | §2.1 | fork 基本用法 | `make fork_test` |
| `fork_fd_test.c` | §2.1 | 文件描述符继承 | `make fork_fd_test` |
| `process_test.c` | §2.1 | fork 后地址空间独立 | `make process_test` |
| `execve_test.c` | §2.2 | execve 程序替换 | `make execve_test` |
| `fork_execve_test.c` | §2.2 | fork+execve 组合 | `make fork_execve_test` |
| `pstree_test.c` | §2.2 | 进程树观察 | `make pstree_test` |
| `waitpid_test.c` | §2.3 | waitpid 等待子进程 | `make waitpid_test` |
| `orphan_process_test.c` | §1.2 | 孤儿进程 | `make orphan_process_test` |
| `system_test.c` | §2.5 | system 执行命令 | `make system_test` |
| `perror_test.c` | §2.7 | perror 错误输出 | `make perror_test` |
| `errno_test.c` | §2.7 | errno 全局变量 | `make errno_test` |
| `unnamed_pipe_test.c` | §3.1 | 匿名管道（父子） | `make unnamed_pipe_test` |
| `my_unnamed_pipe_test.c` | §3.1 | 匿名管道（兄弟） | `make my_unnamed_pipe_test` |
| `fifo_write.c` | §3.2 | 有名管道写端 | 终端1: `make fifo_write && ./fifo_write` |
| `fifo_read.c` | §3.2 | 有名管道读端 | 终端2: `make fifo_read && ./fifo_read` |
| `shared_memory.c` | §3.3 | mmap 匿名共享内存 | `make shared_memory` |
| `shared_memory2.c` | §3.3 | POSIX 共享内存 | `make shared_memory2` |
| `producer.c` | §3.4 | 消息队列生产者 | `make producer && ./producer` |
| `consumer.c` | §3.4 | 消息队列消费者 | `make consumer && ./consumer` |
| `father_son_mq_test.c` | §3.4 | 父子消息队列 | `make father_son_mq_test` |
| `signal_test.c` | §4.3 | signal 捕获 SIGINT | `make signal_test` |
| `erlou.c` | §2.2 | execve 目标程序 | 辅助文件 |
| `erlou_block.c` | §1.2 | 孤儿进程辅助 | 辅助文件 |
