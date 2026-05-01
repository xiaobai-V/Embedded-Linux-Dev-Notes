#include <stdio.h>

int main()
{
    // 打开文件
    FILE *ioFile = fopen("io.txt", "r");
    if (ioFile == NULL)
    {
        perror("fopen");
    }

    // fgetc读取文件
    /**
     * @brief 简述：读取一个字节（不是字符）
     *
     * @def 函数原型：int fgetc (FILE *__stream)
     *
     * @param FILE *__stream：
     *      含义：文件指针
     *
     * @return 返回值：
     *      成功：读取到的一个字节
     *      文件末尾：EOF
     *      失败：EOF
     *
     * @note 注意事项/易错点：必须用int而不是char接收
     */

    int ch;
    while ((ch = fgetc(ioFile)) != EOF)
    {
        printf("%c", ch);
    }

    if (ferror(ioFile))
    {
        perror("fgetc");
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