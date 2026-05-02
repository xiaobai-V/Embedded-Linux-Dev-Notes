#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    // execve跳转之前
    char *name = "banzhang";
    printf("我是%s %d，我现在在一楼\n", name, getpid());

    // char *argv[] = {"/home/hjy/work/Embedded-Linux-Dev-Notes/process_test/erlou", NULL};
    char *args[] = {"/home/hjy/work/Embedded-Linux-Dev-Notes/process_test/erlou", name, NULL};
    char *envs[] = {NULL};
    // char *envs = {};
    // execve跳转
    /**
     * @brief 简述：跳转到另一个程序
     *
     * @def 函数原型：int execve (const char *__path, char *const __argv[], char *const __envp[])
     *
     * @param const char *__path：
     *      含义：执行程序的路径
     *
     * @param char *const __argv[]：
     *      含义：传入的参数
     *      第一个参数固定是程序的名称，也就是程序的路径
     *      第二个是传入的参数
     *      最后一个参数一定是NULL
     *
     * @param char *const __envp[]：
     *      含义：环境变量
     *      key=value
     *      最后一个参数一定是NULL
     *
     * @return 返回值：
     *      成功：没有返回值
     *      失败：-1
     *
     * @note 注意事项/易错点：
     */
    int re = execve(args[0], args, envs);
    if (re == -1)
    {
        printf("你没机会上二楼\n");
        return -1;
    }
    return 0;
}