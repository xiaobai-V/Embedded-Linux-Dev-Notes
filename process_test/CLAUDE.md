[根目录](../CLAUDE.md) > **process_test**

# process_test

## 模块职责

学习和实践 Linux 进程管理相关的系统调用和库函数，包括进程创建(fork)、程序替换(execve)、命令执行(system)，以及进程间文件描述符的继承与共享写入。

## 入口与启动

每个 `.c` 文件均为独立可执行程序，通过 Makefile 中的对应目标编译运行:

```bash
make fork_test      # 测试 fork() 创建子进程
make fork_fd_test   # 测试 fork() 后文件描述符继承与共享写入
make system_test    # 测试 system() 执行 shell 命令
```

> 注意: `execve_test.c` 目前未被纳入 Makefile 目标，需手动编译:
> ```bash
> gcc -o execve_test execve_test.c && ./execve_test && rm execve_test
> ```

Makefile 使用"编译-运行-清理"一体化模式。

## 对外接口

本模块为学习示例，不对外提供接口。各程序练习的系统调用/库函数:

| 函数 | 头文件 | 说明 |
|------|--------|------|
| `fork()` | `<unistd.h>` | 创建子进程，父进程返回子进程 PID，子进程返回 0 |
| `execve()` | `<unistd.h>` | 替换当前进程映像(当前文件仅有声明，未完成实践) |
| `system()` | `<stdlib.h>` | 执行 shell 命令 |
| `getpid()` | `<unistd.h>` | 获取当前进程 PID |
| `getppid()` | `<unistd.h>` | 获取父进程 PID |
| `open()` | `<fcntl.h>` | 系统级文件打开(Low-level IO) |
| `write()` | `<unistd.h>` | 系统级文件写入 |
| `sleep()` | `<unistd.h>` | 进程休眠 |

## 关键依赖与配置

- **编译器**: GCC(通过 Makefile 中 `CC := gcc` 定义)
- **系统头文件**: `<stdio.h>`、`<unistd.h>`、`<sys/types.h>`、`<stdlib.h>`、`<fcntl.h>`、`<sys/stat.h>`、`<string.h>`
- **测试文件**: `io.txt`(fork_fd_test 使用，演示进程间共享文件写入)

## 数据模型

本模块涉及的核心概念:

- **进程创建**: `fork()` 后父子进程各自独立执行，代码段通过 `pid` 返回值区分执行路径
- **文件描述符继承**: `fork()` 会复制父进程的文件描述符表，父子进程共享同一个文件表项
- **原子写入**: 使用 `O_APPEND` 标志打开文件，内核保证写入的原子性，避免父子进程数据混乱

### fork_fd_test.c 关键设计

```
父进程                          子进程
  |                               |
  |--- open("io.txt", O_APPEND) --|
  |                               |
  |--- fork() -------------------|
  |                               |
  |  sleep(1)                     |  strcpy(buffer, "子进程数据")
  |  strcpy(buffer, "子进程数据")  |  write(fd, buffer, ...)
  |  write(fd, buffer, ...)       |
  |                               |
  |--- close(fd) --------------- |
```

## 测试与质量

- 无自动化测试框架
- `fork_test.c`: 观察控制台输出，验证父子进程的 PID 关系
- `fork_fd_test.c`: 观察 `io.txt` 文件内容，验证父子进程的数据写入
- `system_test.c`: 观察命令执行结果(默认执行 `ping -c 100 www.atguigu.com`)

## 常见问题 (FAQ)

**Q: fork_fd_test.c 中为什么父进程要 sleep(1)?**
A: 为了让子进程先写入，演示父子进程写入顺序。由于使用了 `O_APPEND` 模式，即使写入顺序不同也不会造成数据混乱。

**Q: execve_test.c 为什么没有被纳入 Makefile?**
A: 该文件可能尚未完成实践编写，或者需要特定的命令行参数才能正确运行。当前文件内容为空(仅有一个空行)。

**Q: system_test.c 的 ping 命令会执行很久?**
A: 默认执行 100 次 ping(`-c 100`)，可以修改源码中的参数减少次数，或按 Ctrl+C 中断。

## 相关文件清单

```
process_test/
  Makefile          -- 构建脚本(编译-运行-清理一体化)
  fork_test.c       -- fork() 创建子进程示例
  fork_fd_test.c    -- fork() 后文件描述符继承与共享写入示例
  system_test.c     -- system() 执行 shell 命令示例
  execve_test.c     -- execve() 程序替换(未完成)
  io.txt            -- fork_fd_test 的输出文件
```

## 变更记录 (Changelog)

| 日期 | 操作 | 说明 |
|------|------|------|
| 2026-05-02 | 初始化 | 首次生成模块 CLAUDE.md |
