#include <stdio.h>

int main()
{
    /**
     * @brief 打开一个文件
     * @def FILE *fopen (const char *__restrict __filename,
            const char *__restrict __modes)
     * @param char *__restrict __filename: 字符串表示要打开文件的路径和名称
     * @param char *__restrict __modes
            (1)"r": 只读模式 没有文件打开失败
            (2)"w": 只写模式 存在文件写入会清空文件,不存在文件则创建新文件
            (3)"a": 只追加写模式 不会覆盖原有内容 新内容写到末尾，如果文件不存在
            则创建
            (4)"r+": 读写模式 文件必须存在 写入是从头一个一个覆盖
            (5)"w+": 读写模式 可读取,写入同样会清空文件内容，不存在则创建新文件
            (6)"a+": 读写追加模式 可读取,写入从文件末尾开始，如果文件不存在则创建
     * @return FILE * 结构体指针 表示一个文件，如果失败返回NULL
     */
    char *filename = "io.txt";
    FILE *ioFile = fopen(filename, "a+"); // 追加写，可以打开不存在的文件
    if (ioFile == NULL)
    {
        printf("a+不能打开不存在的文件\n");
        return 1;
    }
    else
    {
        printf("a+可以打开不存在的文件\n");
    }

    // 用fputc写入一个字符
    /**
     * @brief 函数功能简述：向文件中写入一个字符
     *
     * @def 函数原型：
     *      int fputc (int __c, FILE *__stream)
     *
     * @param int __c：
     *      含义：写入的字符，AICII
     *
     * @param FILE *__stream：
     *      含义：写入的文件名，写入在哪里和文件打开模式有关系
     *
     * @return int：
     *      成功：返回char的纸
     *      失败：EOF
     *
     * @note 注意事项/易错点：
     */
    char ioChar = 'a';
    int putcR = fputc(ioChar, ioFile);
    if (putcR == EOF)
    {
        perror("fputc");
    }
    else
    {
        printf("写入字符成功%c\n", putcR);
    }

    int result = fclose(ioFile);
    if (result == EOF)
    {
        perror("fclose");
        return 1;
    }
    return 0;
}