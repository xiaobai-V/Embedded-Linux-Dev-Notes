#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    pid_t son_pid = fork();
    if (son_pid > 0)
    {
        waitpid(son_pid, NULL, 0);
    }
    else if (son_pid == 0)
    {
        char *str = malloc(100);
        fgets(str, 100, stdin);
        printf("收到子进程数据: %s\n", str);
        free(str);
        str = NULL;
    }
    else
    {
        perror("fork");
    }
    return 0;
}