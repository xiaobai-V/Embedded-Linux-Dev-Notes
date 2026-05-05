#include <stdio.h>
#include <unistd.h>

int main()
{
    int val = 123;

    __pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
    }

    // 子进程中val的值是321, 地址值是0x7ffd07ce3fe0
    // 父亲进程中val的值是123, 地址值是0x7ffd07ce3fe0
    if (pid == 0)
    {
        // 子进程
        val = 321;
        printf("子进程中val的值是%d, 地址值是%p\n", val, &val);
    }
    else
    {
        // 父进程
        sleep(1);
        printf("父亲进程中val的值是%d, 地址值是%p\n", val, &val);
    }

    return 0;
}
