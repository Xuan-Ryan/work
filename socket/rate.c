#include <stdio.h>
#include <unistd.h>
#define G "\033[0;32m"

int main()
{
    int i = 0;
    char bar[120];
    const char *lable = "|/-\\";
    while (i <= 100)
    {
        printf(G "[%-100s][%d%%][%c]\r", bar, i, lable[i % 4]);
        fflush(stdout);
        bar[i] = '#';
        i++;
        bar[i] = 0;
        usleep(100000);
    }
    printf("\n");
    return 0;
}
