#include <stdio.h>
#include <stdlib.h>
/*
a++  ->>會先執行整個敘述後再將a的值加1

++a  ->>先把a的值加1，再執行整個敘述
*/

int main()
{

  int a = 3,b = 3;
  
  printf("a= %d",a);
  printf(", a++= %d",a++);
  printf(", a= %d\n",a);
  
  printf("b=%d",b);
  printf(", ++b= %d",++b);
  printf(", b= %d\n",b);
 
  system("pause");	 
  return 0;
}