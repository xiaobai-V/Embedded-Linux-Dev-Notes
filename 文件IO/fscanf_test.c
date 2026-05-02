#include <stdio.h>

int main()
{
    // 打开文件
    FILE *userFile = fopen("user.txt", "r");
    if (userFile == NULL)
    {
        perror("fopen");
    }

    // fscanf读取文件
    char name[50];
    int age;
    char wife[50];
    int scanfR;

    /**
     * @brief 简述：格式化读取文件
     *
     * @def 函数原型：int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...)
     *
     * @param FILE *__restrict __stream：
     *      含义：读取的文件
     *
     * @param const char *__restrict __format：
     *      含义：格式化读取的表达式
     *
     * @param ...：
     *      含义：可变参数列表
     *
     * @return 返回值：
     *      成功：参数的个数
     *      失败：返回0
     *      报错或者结束：EOF
     *
     * @note 注意事项/易错点：
     */
    while (fscanf(userFile, "%s %d %s\n", name, &age, wife) != EOF)
    {
        printf("%s在%d岁的时候爱上了%s\n", name, age, wife);
    }

    // 关闭文件
    int result = fclose(userFile);
    if (result != 0)
    {
        perror("fclose");
        return 1;
    }

    return 0;
}