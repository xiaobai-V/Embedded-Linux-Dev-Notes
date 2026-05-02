#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    // malloc
    char *ch = malloc(sizeof(char) * 100);
    // 标准输入
    // fgets从标准输入中读取数据;
    fgets(ch, 100, stdin);
    printf("你好,%s", ch);

    // 标准输出
    fputs(ch, stdout); // 不会换行

    // 错误输出
    fputs(ch, stderr); // 文件描述符合标准输出不一样

    return 0;
}
