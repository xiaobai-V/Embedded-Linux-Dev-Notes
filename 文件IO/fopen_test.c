#include <stdio.h>

int main()
{
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
    fclose(file);
    return 0;
}