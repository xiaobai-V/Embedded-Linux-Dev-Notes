#include <stdio.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
    fopen("/opt", "a+");
    printf("errno: %d\n", errno);
    perror("文件打开出现问题");
    return 0;
}
