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
    FILE *file = fopen("example.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    else
    {
        printf("File opened successfully.\n");
    }

    /**
     * @brief 简述：关闭文件
     *
     * @def 函数原型：
     *      int fclose (FILE *__stream)
     *
     * @param FILE *__stream：
     *      含义：要关闭的文件结构体指针
     *
     * @return int：
     *      成功：0
     *      失败：EOF
     *
     * @note 注意事项/易错点：
     */
    int result = fclose(file);
    if (result == EOF)
    {
        perror("fclose");
        return 1;
    }
    return 0;
}