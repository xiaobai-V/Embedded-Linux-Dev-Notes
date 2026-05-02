#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    char *name = "老学员";
    printf("%s %d在一楼精进\n", name, getpid());
    __pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        // 新学员子进程
        // 子进程跳转执行新的任务
        char *newName = "ergou";
        char *args[] = {"/home/hjy/work/Embedded-Linux-Dev-Notes/process_test/erlou", newName, NULL};
        char *envs[] = {NULL};
        int re = execve(args[0], args, envs);
        if (re == -1)
        {
            perror("execve");
            return -1;
        }
    }
    else
    {
        // 老学员进程
        printf("老学员%d邀请完%d之后还是在一楼学习\n", getpid(), pid);
    }

    return 0;
}
