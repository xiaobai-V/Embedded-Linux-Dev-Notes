#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/**
 *
 *
 *
 */

int main(int argc, char const *argv[])
{
    // 调用fork之前 代码都在父进程中运行
    printf("老学员%d正在学习尚硅谷的嵌入式Linux应用开发教程\n", getpid());

    // 使用fork创建子进程
    /**
     * 不需要传参
     * return: int 进程号
     *      (1): 父进程中 返回子进程的PID
     *      (2)：子进程中 显示为0
     *      (3): 出现错误显示-1
     */

    // extern __pid_t fork(void) __THROWNL;
    pid_t pid = fork();

    // 从fork之后，所有的代码都是在父子进程中各自执行一次的
    // printf("%d\n", pid);

    if (pid < 0)
    {
        printf("子进程创建失败！");
    }
    else if (pid == 0)
    {
        // 执行单独的子进程的代码
        printf("新学员%d加入成功， 他是老学员%d推荐的\n", getpid(), getppid());
    }
    else
    {
        // 执行单独的父进程代码
        printf("老学员%d继续深造，他推荐了%d\n", getpid(), pid);
    }

    return 0;
}