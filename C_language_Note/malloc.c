#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main()
{
int *p2 = (int*)malloc(10);
*p2 = 2020;
printf("%d\n",*p2);
printf("%p\n",&p2);
p2 = (int*)malloc(20);  //有沒有 沒差別
p2[0] = 50;
p2[1] = 30;
printf("%d %d\n",*p2,p2[1]);
printf("%p\n",&p2);
free(p2);

 }
