#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "write_file.txt"
void write(int i)
{
    char str[4] = {};
    FILE *fp = fopen(FILENAME, "r+");
    sprintf(str, "%d", i);
    //printf("str = %s\n", str);
    fwrite(str, 1, strlen(str), fp);
    fclose(fp);
}

int main()
{

    int i = 0;
    while (i < 101)
    {
        write(i);
        printf("i = %d\n", i);
        i++;
        sleep(1);
    }
    printf("write finish\n");
}