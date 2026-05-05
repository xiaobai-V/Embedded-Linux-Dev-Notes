# Linux 多线程编程总结

> 本文档系统总结 Linux POSIX 线程（pthread）编程的核心知识点，涵盖线程控制、线程同步、线程池等内容。
> 所有代码示例对应 `thread_test/` 目录下的实践文件。

---

## 一、Linux 线程简介

### 1.1 什么是线程

线程（Thread）是操作系统能够进行运算调度的最小单位，也被称为**轻量级进程（LWP）**。一个进程可以包含多个线程，它们共享进程的资源但独立执行。

### 1.2 线程 vs 进程

| 特性       | 进程                           | 线程                           |
| ---------- | ------------------------------ | ------------------------------ |
| 创建开销   | 大（需要复制页表、文件描述符） | 小（共享地址空间）             |
| 通信方式   | 管道/消息队列/共享内存/信号量  | 直接读写全局变量（需同步）     |
| 切换开销   | 大（切换地址空间）             | 小（共享地址空间）             |
| 崩溃影响   | 进程间相互隔离                 | 一个线程崩溃通常导致整个进程退出 |

### 1.3 线程的共享资源与私有资源

**共享资源（同一进程内所有线程共享）：**
- 代码段（.text）
- 全局变量 / 堆内存
- 文件描述符表
- 当前工作目录、用户 ID 等

**私有资源（每个线程独有）：**
- 线程 ID（`pthread_t`）
- 栈空间（默认 8MB）
- 寄存器组（包括程序计数器、栈指针）
- errno 变量
- 信号屏蔽字

### 1.4 编译要求

POSIX 线程库（libpthread）不是 C 运行库的一部分，编译时必须显式链接：

```bash
gcc program.c -o program -lpthread
```

---

## 二、线程控制

### 2.1 线程创建

```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
```

| 参数           | 说明                                                 |
| -------------- | ---------------------------------------------------- |
| `thread`       | [输出] 成功后存储新线程标识符                         |
| `attr`         | 线程属性，`NULL` = 默认（可 joinable，栈大小 8MB）   |
| `start_routine`| 线程入口函数，签名 `void *(*)(void *)`               |
| `arg`          | 传递给入口函数的参数，不需要时传 `NULL`              |
| **返回值**     | 成功返回 0，失败返回错误号（不设置 errno）           |

> 参考：`create_test.c`

### 2.2 线程终止

线程有三种终止方式：

| 方式               | 适用场景               | 返回值                       |
| ------------------ | ---------------------- | ---------------------------- |
| `return value`     | 在入口函数中直接退出   | value 被 `pthread_join` 接收 |
| `pthread_exit()`   | 在任意函数深度退出线程 | 参数被 `pthread_join` 接收   |
| `pthread_cancel()` | 由其他线程请求取消     | `PTHREAD_CANCELED`           |

```c
void pthread_exit(void *retval);
```
- `retval`：线程返回值，可被 `pthread_join` 的第二个参数接收
- 与 `return` 的区别：在嵌套调用中，`return` 只退出当前函数，`pthread_exit()` 立即终止整个线程

> 参考：`terminate_test.c`

### 2.3 线程等待与资源回收

```c
int pthread_join(pthread_t thread, void **retval);
```

| 参数      | 说明                                      |
| --------- | ----------------------------------------- |
| `thread`  | 目标线程标识符                            |
| `retval`  | [输出] 接收线程返回值，不关心传 `NULL`    |
| **返回值**| 成功返回 0，失败返回错误号                |

- 类似进程的 `waitpid()`，不调用会造成线程资源泄漏
- 只能等待 **joinable** 状态的线程

> 参考：`create_test.c`、`terminate_test.c`

### 2.4 线程分离

```c
int pthread_detach(pthread_t thread);
```

线程有两种状态：

| 状态            | 资源回收     | 可否 join |
| --------------- | ------------ | --------- |
| Joinable（默认）| 必须 `join`  | 可以      |
| Detached        | 系统自动回收 | 不可以    |

两种分离方式：
1. 主线程调用 `pthread_detach(tid)` —— 推荐
2. 线程内部调用 `pthread_detach(pthread_self())`

```c
pthread_t pthread_self(void);  // 获取当前线程 ID
```

> 参考：`pthread_detach_test.c`

### 2.5 线程取消

#### 2.5.1 发送取消请求

```c
int pthread_cancel(pthread_t thread);
```

- 仅表示请求已发送，不代表线程已终止
- 实际取消行为取决于目标线程的**取消状态**和**取消类型**

#### 2.5.2 取消类型

```c
int pthread_setcanceltype(int type, int *oldtype);
```

| 类型                          | 说明                     | 安全性 |
| ----------------------------- | ------------------------ | ------ |
| `PTHREAD_CANCEL_DEFERRED`     | 延迟到取消点才生效（默认）| 安全  |
| `PTHREAD_CANCEL_ASYNCHRONOUS` | 任意时刻立即取消          | 危险  |

**常见取消点：** `sleep`、`read`、`write`、`printf`、`pthread_cond_wait`、`pause` 等

> 参考：`pthread_cancel_deferred_test.c`（延迟取消）、`pthread_cancel_async_test.c`（异步取消）

#### 2.5.3 取消状态

```c
int pthread_setcancelstate(int state, int *oldstate);
```

| 状态                        | 说明                   |
| --------------------------- | ---------------------- |
| `PTHREAD_CANCEL_ENABLE`     | 允许取消（默认）       |
| `PTHREAD_CANCEL_DISABLE`    | 禁用取消（请求挂起）   |

推荐的临界区保护模式：

```c
pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
// ... 临界区操作 ...
pthread_setcancelstate(oldstate, NULL);
pthread_testcancel();  // 立即检查挂起的取消请求
```

> 参考：`pthread_cancel_disabled_test.c`

#### 2.5.4 清理函数

```c
void pthread_cleanup_push(void (*routine)(void *), void *arg);
void pthread_cleanup_pop(int execute);
```

- 清理函数按**栈式 LIFO** 顺序调用（后注册先调用）
- 必须**成对出现**（它们是宏，用 `{ }` 配对）
- 线程被取消时自动依次弹出并执行所有已注册的清理函数

> 参考：`pthread_cancel_deferred_test.c`

---

## 三、线程同步

### 3.1 竞态条件

当多个线程并发访问共享资源，且至少一个线程进行写操作时，执行结果取决于线程的调度顺序，这就是**竞态条件（Race Condition）**。

典型表现：20000 个线程对同一变量 `+1`，预期结果 20000，实际结果不确定。

> 参考：`race_condition_test.c`

### 3.2 互斥锁（Mutex）

互斥锁是最基本的同步原语，保证同一时刻只有一个线程进入临界区。

#### 初始化

```c
// 方式1：静态初始化（推荐简单场景）
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// 方式2：动态初始化
pthread_mutex_t mutex;
pthread_mutex_init(&mutex, NULL);
// 使用完毕后：
pthread_mutex_destroy(&mutex);
```

#### 加锁与解锁

```c
int pthread_mutex_lock(pthread_mutex_t *mutex);    // 阻塞加锁
int pthread_mutex_trylock(pthread_mutex_t *mutex);  // 非阻塞加锁
int pthread_mutex_unlock(pthread_mutex_t *mutex);   // 解锁
```

**基本使用模式：**

```c
pthread_mutex_lock(&mutex);
// ... 临界区操作 ...
pthread_mutex_unlock(&mutex);
```

> 参考：`mutex_test.c`

### 3.3 读写锁（RWLock）

读写锁区分**读锁（共享锁）**和**写锁（排他锁）**：
- 多个读者可以同时持有读锁
- 写锁独占，与读锁、其他写锁互斥
- 适用于**读多写少**的场景

#### API

```c
int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);   // 读锁
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);   // 写锁
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);   // 解锁
```

#### 写饥饿问题

默认情况下，读写锁是**读者优先**：只要有一个读者持有读锁，其他读者可以继续获取读锁，写者被阻塞。如果读操作频繁，写者可能长时间得不到执行——这就是**写饥饿**。

**解决方案：** 设置写者优先

```c
pthread_rwlockattr_t attr;
pthread_rwlockattr_init(&attr);
// 设置写优先属性
pthread_rwlock_init(&rwlock, &attr);
pthread_rwlockattr_destroy(&attr);
```

> 参考：
> - 基本使用：`rwlock_test.c`、`rwlock_random_order.c`
> - 写者未加锁的危险：`rwlock_test_writter_unlock.c`
> - 写饥饿问题：`rwlock_write_hungry.c`
> - 写饥饿解决：`rwlock_write_hungry_solved.c`

### 3.4 自旋锁（Spinlock）

自旋锁与互斥锁的用途类似，都是保护临界区，但**阻塞机制不同**：

- **互斥锁**：获取失败时线程**睡眠**，让出 CPU
- **自旋锁**：获取失败时线程**忙等待（自旋）**，不断尝试获取

**适用场景：**
- 临界区非常短（通常只有几条指令）
- 在中断上下文中使用（不能睡眠）
- 多处理器系统（单处理器自旋无意义）

**注意事项：**
- 持有自旋锁时不能睡眠/阻塞，否则会导致其他线程无限自旋
- 不适合 I/O 操作或长临界区
- Linux 内核中广泛使用，用户态一般用互斥锁即可

POSIX 标准未提供自旋锁 API（`pthread_spinlock_t` 属于可选扩展），本目录暂无对应示例。

### 3.5 条件变量（Condition Variable）

条件变量用于线程间**通知-等待**机制，必须与互斥锁配合使用。解决了"忙等待"浪费 CPU 的问题。

#### API

```c
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);      // 等待
int pthread_cond_signal(pthread_cond_t *cond);   // 唤醒一个等待线程
int pthread_cond_broadcast(pthread_cond_t *cond); // 唤醒所有等待线程
```

#### 标准使用模式

**等待方（消费者）：**

```c
pthread_mutex_lock(&mutex);
while (condition_not_met) {          // 必须用 while，不能用 if
    pthread_cond_wait(&cond, &mutex); // 原子：解锁+睡眠，被唤醒后重新加锁
}
// ... 处理数据 ...
pthread_mutex_unlock(&mutex);
```

**通知方（生产者）：**

```c
pthread_mutex_lock(&mutex);
// ... 修改共享数据 ...
pthread_cond_signal(&cond);  // 唤醒等待线程
pthread_mutex_unlock(&mutex);
```

#### 为什么必须用 while 而不是 if？

1. **虚假唤醒**：POSIX 允许 `pthread_cond_wait` 在没有 `signal` 的情况下返回
2. **多消费者竞争**：被唤醒时条件可能已被其他消费者处理

#### 生产者-消费者模型

> 参考：`condition_var.c`（经典模型）、`ringbuf_demo.c`（环形缓冲区完整实现）、`thread_ringbuf_test.c`（键盘输入实时回显）

### 3.6 信号量（Semaphore）

信号量是一个**计数器**，用于控制对共享资源的访问。核心操作：
- **P 操作**（`sem_wait`）：计数器 -1，若结果 < 0 则阻塞
- **V 操作**（`sem_post`）：计数器 +1，唤醒等待线程

#### 分类

| 分类维度   | 类型         | 说明                            |
| ---------- | ------------ | ------------------------------- |
| 初始值     | 二值信号量   | 初始值 1，等效互斥锁            |
|            | 计数信号量   | 初始值 > 1，管理多个同类资源    |
| 可见性     | 命名信号量   | 有名字（字符串），可用于进程间  |
|            | 无名信号量   | 存于内存，可用于线程间或进程间  |

#### 无名信号量

```c
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_wait(sem_t *sem);    // P 操作
int sem_post(sem_t *sem);    // V 操作
int sem_destroy(sem_t *sem); // 销毁
int sem_getvalue(sem_t *sem, int *sval); // 获取当前值
```

**`pshared` 参数至关重要：**

| pshared | 含义                       | futex 类型     |
| ------- | -------------------------- | -------------- |
| 0       | 线程间共享                 | FUTEX_PRIVATE  |
| 非 0    | 进程间共享（需放在共享内存）| FUTEX_SHARED  |

> **重要：** 进程间使用无名信号量时，必须将信号量放在共享内存中且 `pshared=1`。如果错误地使用 `pshared=0`，在无竞态时可能碰巧正常工作，但一旦触发阻塞路径就会死锁（PRIVATE futex 跨进程唤醒失效）。
>
> 参考：`unnamed_sem_bin_process_illegal.c`（错误示例及详细解释）

**线程间使用：**
> - 二值信号量作互斥锁：`unnamed_sem_bin_thread.c`
> - 二值信号量 + 条件变量：`unnamed_sem_bin_thread_condition.c`
> - 计数信号量生产者-消费者：`unnamed_sem_count_thread.c`

**进程间使用：**
> - 正确的进程间同步：`unnamed_sem_bin_process.c`
> - 错误的 pshared 用法：`unnamed_sem_bin_process_illegal.c`
> - 进程间条件同步：`unnamed_sem_bin_process_condition.c`
> - 独立信号量模式：`unnamed_sem_bin_process_isolate.c`
> - 进程间生产者-消费者：`unnamed_sem_count_process.c`

#### 命名信号量

命名信号量通过字符串名字标识，存在于文件系统中（通常在 `/dev/shm/`），适用于**无亲缘关系进程间同步**。

```c
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
```

- `sem_open`：创建或打开命名信号量
- `sem_close`：关闭引用（每个进程都应调用）
- `sem_unlink`：删除信号量（只调用一次，通常由创建者执行）

> 参考：`named_sem_bin.c`（二值信号量）、`named_sem_count.c`（计数信号量）

#### 信号量实现环形缓冲区

使用三个信号量实现完整的生产者-消费者模型：

```
sem_empty  — 空槽位数（初始 = BUF_SIZE，控制生产者不溢出）
sem_full   — 数据个数  （初始 = 0，控制消费者不欠读）
sem_mutex  — 互斥访问  （初始 = 1，保护缓冲区结构）
```

> 参考：`sem_ringbuf_test.c`

---

## 四、线程池

### 4.1 概念

线程池预先创建一组线程，任务到来时从池中取线程执行，任务完成后线程不销毁而是回到池中等待下一个任务。

**优势：**
- 避免频繁创建/销毁线程的开销
- 控制并发线程数量，防止资源耗尽
- 任务提交与执行解耦

### 4.2 GLib 线程池

本项目使用 GLib 库的线程池实现：

```c
// 创建线程池
GThreadPool *pool = g_thread_pool_new(task_func, user_data,
                                       max_threads, exclusive, NULL);

// 提交任务（data 会传给 task_func 的第一个参数）
g_thread_pool_push(pool, data, NULL);

// 等待所有任务完成并释放线程池
g_thread_pool_free(pool, wait_pending, immediate);
```

| 参数            | 说明                          |
| --------------- | ----------------------------- |
| `max_threads`   | 最大线程数                    |
| `exclusive`     | `TRUE` = 线程专属于该池       |
| `wait_pending`  | `TRUE` = 等待未完成任务完成   |
| `immediate`     | `TRUE` = 立即停止（不等待）   |

编译需要链接 GLib：

```bash
gcc thread_pool_test.c -o thread_pool_test $(pkg-config --cflags --libs glib-2.0)
```

> 参考：`thread_pool_test.c`

---

## 五、同步机制对比

| 机制     | 核心特性                       | 适用场景               | 复杂度 |
| -------- | ------------------------------ | ---------------------- | ------ |
| 互斥锁   | 排他访问，睡眠等待             | 通用临界区保护         | 低     |
| 读写锁   | 读共享 / 写独占                | 读多写少               | 中     |
| 自旋锁   | 排他访问，忙等待               | 极短临界区 / 中断上下文 | 低     |
| 条件变量 | 等待-通知机制，需配合互斥锁    | 复杂条件等待           | 高     |
| 信号量   | 计数器，可跨进程               | 资源计数 / 生产者-消费者 | 中   |

**选择建议：**

- 简单互斥 → 互斥锁
- 读多写少 → 读写锁
- 线程间通知 → 条件变量 + 互斥锁
- 资源计数 / 跨进程同步 → 信号量
- 极短临界区 / 内核态 → 自旋锁

---

## 六、常见陷阱与最佳实践

### 6.1 常见陷阱

1. **忘记 `pthread_join`**：joinable 线程终止后不 join，资源泄漏（类似僵尸进程）
2. **互斥锁不解锁**：异常路径忘记 unlock → 死锁
3. **条件变量用 if 不用 while**：虚假唤醒导致逻辑错误
4. **信号量 pshared 用错**：进程间必须 `pshared=1`，否则可能死锁
5. **异步取消**：可能在任意指令处终止线程，导致资源泄漏或死锁
6. **返回栈上局部变量指针**：线程结束后栈帧失效，成为悬空指针

### 6.2 最佳实践

1. 优先使用静态初始化宏（`PTHREAD_MUTEX_INITIALIZER` 等）
2. 条件变量等待必须用 `while` 循环检查条件
3. 信号量 P 操作顺序要注意，避免死锁（先获取资源信号量，再获取互斥信号量）
4. 生产环境中避免使用异步取消
5. 线程返回值使用堆内存或全局变量，不要返回栈上局部变量
6. 临界区尽量短，减少锁持有时间
