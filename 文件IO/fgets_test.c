#include <stdio.h>

int main()
{
    // 打开文件
    FILE *ioFile = fopen("io.txt", "r");
    if (ioFile == NULL)
    {
        perror("fopen");
    }

    // fgets读取文件
    /**
     * @brief 简述：
     *
     * @def 函数原型：char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     *
     * @param char *__restrict __s：
     *      含义：接收读取的字符串
     *
     * @param int __n：
     *      含义：能够读取的长度
     *
     * @param FILE *__restrict __stream：
     *      含义：读取的文件
     *
     * @return char *：
     *      成功：返回字符串
     *      失败：NULL
     *
     * @note 注意事项/易错点：
     */

    char buffer[100];
    while (fgets(buffer, sizeof(buffer), ioFile))
    {
        printf("%s", buffer);
    }

    if (ferror(ioFile))
    {
        perror("fgets");
    }

    // 关闭文件
    int result = fclose(ioFile);
    if (result != 0)
    {
        perror("fclose");
        return 1;
    }

    return 0;
}