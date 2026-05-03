/**
 * @file terminate_test.c
 * @brief 线程终止与返回值获取
 *
 * 知识点：
 *   - 线程的3种终止方式：return / pthread_exit() / pthread_cancel()
 *   - pthread_join() 获取线程返回值
 *   - 返回值可以是普通指针，也可以是结构体指针
 *
 * 编译：gcc terminate_test.c -o terminate_test -lpthread
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief 计算结果结构体 —— 演示线程通过结构体返回多个值
 */
typedef struct {
    int sum;
    double avg;
    int count;
} Result;

/**
 * @brief 方式1：通过 return 终止线程，返回动态分配的结构体
 *
 * @param arg 传入 int 数组的首元素地址（数组长度约定为5）
 * @return Result* 动态分配的计算结果（由 pthread_join 接收）
 *
 * @note 返回的指针必须是堆内存或全局变量，不能是栈上局部变量
 *       因为线程结束后栈帧就失效了，访问会成为悬空指针。
 */
void *calc_return(void *arg)
{
    int *nums = (int *)arg;

    // 在堆上分配结果（返回后 main 中 free）
    Result *res = (Result *)malloc(sizeof(Result));
    res->sum = 0;
    res->count = 5;

    for (int i = 0; i < res->count; i++)
    {
        res->sum += nums[i];
    }
    res->avg = (double)res->sum / res->count;

    // 方式1：return 终止线程，返回值会被 pthread_join 的 retval 接收
    return (void *)res;
}

/**
 * @brief 方式2：通过 pthread_exit() 终止线程
 *
 * @param arg 未使用
 * @return 不返回（pthread_exit 不返回调用者）
 *
 * @fn pthread_exit(void *retval)
 * @brief 显式终止当前线程
 * @param retval 线程的返回值，可被 pthread_join 的 retval 参数接收
 * @note 与 return 的区别：
 *       - return 仅退出当前函数，如果在嵌套调用中只退出最内层函数
 *       - pthread_exit() 在任何调用深度都能立即终止整个线程
 *       - 在线程入口函数中，return 等价于 pthread_exit()
 */
void *thread_exit_demo(void *arg)
{
    (void)arg;
    printf("[thread_exit_demo] 即将通过 pthread_exit 退出\n");

    // 方式2：pthread_exit 终止线程
    pthread_exit((void *)"我是 pthread_exit 的返回值");

    // 这行代码永远不会执行
    printf("这行不会被打印\n");
    return NULL;
}

int main(void)
{
    pthread_t tid1, tid2;
    int nums[5] = {10, 20, 30, 40, 50};

    /**************************************************************************
     * 演示1：return 终止 + 获取结构体返回值
     **************************************************************************/
    printf("===== 演示1：return 终止线程 =====\n");

    pthread_create(&tid1, NULL, calc_return, (void *)nums);

    /**
     * pthread_join 的第二个参数 retval 是 "指向指针的指针" (void **)
     * 线程返回的 void* 会被写入 *retval
     */
    void *ret_ptr = NULL;
    pthread_join(tid1, &ret_ptr);

    // 将 void* 转回 Result* 即可使用
    Result *res = (Result *)ret_ptr;
    printf("计算结果: sum=%d, avg=%.1f, count=%d\n", res->sum, res->avg, res->count);
    free(res); // 释放线程中 malloc 的内存

    /**************************************************************************
     * 演示2：pthread_exit 终止 + 获取字符串返回值
     **************************************************************************/
    printf("\n===== 演示2：pthread_exit 终止线程 =====\n");

    pthread_create(&tid2, NULL, thread_exit_demo, NULL);

    void *ret_str = NULL;
    pthread_join(tid2, &ret_str);
    printf("收到返回值: \"%s\"\n", (char *)ret_str);

    /**************************************************************************
     * 三种终止方式对比：
     *
     * 方式            | 适用场景                      | 返回值
     * --------------- | ----------------------------- | -------------------
     * return value    | 在入口函数中直接退出            | value 被 join 接收
     * pthread_exit()  | 在任意函数深度退出线程          | 参数被 join 接收
     * pthread_cancel()| 由其他线程请求取消（后续示例详解）| PTHREAD_CANCELED
     **************************************************************************/
    printf("\n程序正常退出\n");
    return 0;
}
