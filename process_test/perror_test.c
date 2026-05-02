#include <stdio.h>

int main(int argc, char const *argv[])
{
    fopen("none.txt", "r");
    // 出现错误，设置全局变量errno，

    perror("open");

    return 0;
}
