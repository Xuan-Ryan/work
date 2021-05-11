#include <stdio.h>
#include <string.h> //#include <string.h>  計算字串長度
#include <stdlib.h> //#include <stdlib.h>  0~100亂數
#include <time.h>  // #include <time.h>  系統時間
int main(){

    char s[] = "You";
    char ss[] = "Yo";
    int aa;
    aa = strlen(s);  //#include <string.h>  計算字串長度
    printf("%d\n", aa);

printf("---------------randon----------------------\n");
	srand(time(NULL));  // #include <time.h>  系統時間
	int a = (rand()%100)+1; //#include <stdlib.h>  1~100亂數
	printf("%d\n",a);

printf("-----------------轉型態--------------------\n");
	int value = 123;
	char buffer[4];
	char buffer2[4];
	ltoa(value, buffer, 10);
	printf("%s\n",buffer);   //長整數轉為任意進制字串
	
	puts(buffer);
	//int aaa = atoi(buffer); 
	printf("%d\n",atoi(buffer));   //長整數轉為任意進制字串

	sprintf(buffer2,"%d",value); //整數轉為任意進制字串
	puts(buffer2);
	

printf("----------------比較字串---------------------\n");
	int bb = strcmp(s, ss);  // #include <string.h> 比較字串 誰大
	printf("%d\n",bb);
printf("-----------------數字分開相加--------------------\n");
	int digits = 19, sum = 0;
	for (digits = abs(digits); digits; digits /= 10) 
		sum += (digits % 10); ///利用這種方式可以分開19>>1 9 相家
	printf("%d\n",sum);
return 0;
}