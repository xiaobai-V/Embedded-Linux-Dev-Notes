#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    // fork之前
    printf("老学员在校区\n");
    int subprocessState;
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return -1;
    }
    else if (pid == 0)
    {
        // 新学员
        char *args[] = {"/usr/bin/ping", "-c", "10", "www.atguigu.com", NULL};
        char *envs[] = {NULL};
        printf("新学员%d联系海哥10次\n", getpid());
        int re = execve(args[0], args, envs);
        if (re == -1)
        {
            perror("execve");
            return -1;
        }
    }
    else
    {
        // 老学员
        printf("老学员%d等待新学员%d联系\n", getpid(), pid);
        waitpid(pid, &subprocessState, 0);
    }
    // 只有老学员会执行，新学员通过execve执行到其他程序了
    printf("老学员等待新学员联系成功\n");

    return 0;
}
