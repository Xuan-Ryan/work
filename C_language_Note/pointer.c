#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{
	int a = 5;
	int *prt;
	prt = &a;
	//printf("&ad = %d\n",prt);
	printf("*prt = %d\n",*prt);  //*抓prt內容
	printf("prt = %p\n",prt);  //*抓prt內容的a位置
	printf("&prt = %p\n",&prt);//*抓prt的位置

	printf("&a = %p\n",&a);//*抓a位置
	printf("a = %d\n",a); //*抓a內容
printf("--------------point char-----------------\n");
	//printf("%s",strin);  如果是字串可以用S抓
	//printf("%s",&strin[0]); 
	//printf("%d\n",*strin);
    char str[] = "http://c.biancheng.net";
    char *pstr = str;
    int len = strlen(str), i;
    //使用*(pstr+i)
    for(i=0; i<len; i++){
        printf("%c", *(pstr+i));
    }
    printf("\n");
    //使用pstr[i]
    for(i=0; i<len; i++){
        printf("%c", pstr[i]);
    }
    printf("\n");
    //使用*(str+i) 
    for(i=0; i<len; i++){
        printf("%c", *(str+i));
    }
    printf("\n");
//s字串不能*   %p>>*pstr 
    printf("%s\n",str);  // = pstr
    printf("%c\n",*pstr); //字串地一位
    printf("%s\n",&pstr[0]);
    printf("%s\n",pstr); // = str
    printf("%p\n",pstr); // = str的位置
    printf("%p\n",&pstr); // = pstr本身的初始位置
    printf("%p\n",&str); // = str本身的位置

    printf("%c\n",*++pstr); // = a本身第二位
    printf("%p\n",pstr); // = str的位置
    printf("%c\n",*str); // = a本身第二位
    printf("%p\n",str); // = a的位置

printf("--------------double_pointer------------------\n");
    int aa = 10;
    int *ptr1 = &aa;
    int **ptr2 = &ptr1;

    printf("aa_address: %p\n", &aa);
    printf("ptr1_address: %p\n", &ptr1);
    printf("ptr2_address: %p\n", &ptr2);

    printf("ptr1_value: %p\n", ptr1);
    printf("ptr2_value: %p\n", ptr2);
    printf("*ptr2_value: %p\n", *ptr2);

    printf("a_value:: %d\n", aa);
    printf("*ptr1_value:(point to variable aa): %d\n", *ptr1);
    printf("**ptr2_value:(point to variable aa): %d\n", **ptr2);
printf("--------------pointer detail------------------\n");
    char *s[] = {"111","222","678","444"};
    char **ptr[]={s+3,s+2,s+1,s},***p;
    p = ptr;
    ++p;
    printf("%s\n",**p+1);  //只顯示兩個 
    printf("%s\n",**(p+2));  //顯示最後一個 s / 和**p++相等 一樣是加記憶體位置
    printf("%c\n",***p);  //只顯示一個
    printf("%c\n",***p++);  //只顯示兩個 
printf("--------------pointer detail2------------------\n");//++與*的運算優先權相同運算從右到左   +1優先權低於++從左到右   括號優先權最高  <關鍵>
    char bb[]="123456789";
    char *b = bb;
    printf("%c\n",*b++); //先印後加1 ； ++是加記憶體位置 +1是數值
    printf("%c\n",*(b++)); //和ba++相等 一樣是加記憶體位置 ；但下一個列印指標位置不變動
    printf("%c\n",*(b+1)); //上個++後指向內容+1  並且下一個列印指標位置不變動
    printf("%c\n",*b+1); //上個相同
    printf("%c\n",*b); 
//++跟+1差別 ++是加記憶體位置 +1是數值 有刮沒刮都相同
printf("---------------------------\n"); 
    printf("%c\n",(*b)++);// a目前=3 印完後 再指標+1 
    printf("%c\n",*b); 
printf("---------------------------\n"); 
    printf("%c\n",*++b);//  ++與*的運算優先權相同
    printf("%c\n",*b); 
printf("---------------------------\n"); 

    printf("%c\n",*(++b)); //相等
    printf("%c\n",*b); 
    printf("%c\n",++*b); //5 +1 上面內容指到5 內容先+1在印出  並且會更改內容
    printf("%c\n",++(*b));  // 上面內容6 + 1   並且會更改內容
    printf("%c\n",*b);
    printf("---------------------------\n"); 
    b = b-3; //指標位置倒退3    等同於b[]位移
    //printf("%c\n",(*b)-3); // 內容扣除3
    printf("%c\n",*b);


    printf(bb);//第三個2因為第11行  7是因為16、17行
    
}