#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **Version_calculation(char *str)
{
    int k, i, j, total;
    int version_num[4];
    int version_calculate[5] = {0};
    char version_buf[4][7] = {0};
    char **version_return = malloc(strlen(str) + 1);
    k = 0;
    for (j = 0; j < 4; j++)
    {
        for (i = k; i <= strlen(str); i++)
        {

            if (str[i] != '.')
            {
                //printf("s[%d] = %c\n", i, buf[i]);
                version_buf[j][i - k] = str[i];
            }
            else
            {
                version_buf[j][i] = '\0';
                k = i + 1;

                //	printf("\n");
                break;
            }
        }
        //printf("version_buf[%d] = %s\n", j, version_buf[j]);
        //printf("======================\n");
    }

    for (i = 0; i < 4; i++)
    {
        *(version_return + i) = version_buf[i];
        printf("version_buf[%d] = %s\n", i, version_buf[i]);
        printf("version_return = %s\n", *version_return);
    }

    return version_return;
}

int main()
{
    char str[13] = "1.0.0.210428";
    char **version;
    int i;
    version = Version_calculation(str);
    /*
    printf("version = %s\n", *version++);
    printf("version = %s\n", *version++);
    printf("version = %s\n", *version++);
    printf("version = %s\n", *version);
    */
    for (i = 0; i < 4; i++)
    {
        printf("version = %s\n", *(version + i));
    }
    free(*(version + 0));
    free(*(version + 1));
}