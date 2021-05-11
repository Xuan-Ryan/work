#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
char *strin = "I  am Chinese";
  char *tmp = malloc(strlen(strin) + 1);
  char *start = tmp;
  
  while (*strin != '\0') 
  {
    if (*strin != ' ') 
    {//反像思考
      *tmp++ = *strin;  // 關鍵 將位置做停留 等於原來的位置，再用另一個指標增加只像下個位置
    }
	printf("%p\n",tmp);  //關鍵
  printf("%p\n",tmp);  //關鍵
    strin++; 
  }
  *tmp = '\0';
  printf("%s\n",start);
  printf("%s\n",tmp); //因tmp++到最後指向所以是空的
return 0;
}

/*  類試
remove("Hi!",1) === "Hi"
remove("Hi!",100) === "Hi"

char *remove(char *strin, int n)
{
    char* str = malloc(strlen(strin)+1);
    char* ptr = str;

    while(*strin)
    {
      if (n && *strin == '!')
        n--;
      else
        *ptr++ = *strin;
      strin++;
    }
    *ptr = '\0';

    return str;
}
*/