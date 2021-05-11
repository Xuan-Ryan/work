#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **Version_calculation(char *str)
{
    char *tmp = malloc(strlen(str) + 10);
    char *start = tmp;
    char **start2 = malloc(strlen(str) + 10);

    while (*str != '\0')
    {
        printf("str = %s\n", str);
        if (*str != '.')
        {
            *tmp = *str;
            printf("*tmp = %s\n", tmp);
            *tmp++;
        }
        else
        {

            *tmp = '\0';
            printf("start = %s\n", start);
            *tmp++;

            *start2 = strdup(start);
            printf("start2 = %s\n", *start2);

            if (*start != '.')
                start++;
            start++;
            *start2++;
        }
        *str++;
        printf("----------------------------------------\n");
    }
    *tmp = '\0';
    *start2 = strdup(start);

    return start2;
}

int main()
{
    char str[13] = "1.0.0.210428";
    char **version_buf;
    char **version2;

    version_buf = Version_calculation(str);
    version_buf = version_buf - 3;
    printf("version_buf = %s\n", *version_buf++);
    printf("version_buf = %s\n", *version_buf++);
    printf("version_buf = %s\n", *version_buf);
}