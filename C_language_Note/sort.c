#include <stdio.h>
#include <stdlib.h>
#define SWAP(x,y) {int t; t = x; x = y; y = t;}
#define LEN(x) sizeof(x) / sizeof(x[0])
void bubble(int s[],int length);
void selection(int a[],int length);
void insertion(int a[],int length);
void quickSort(int number[], int left, int right);
int main()
{
	int s[] = {12,51,62,11,99,24,19,9};
	int length = sizeof(s) / sizeof(s[0]);
	printf("%d\n",length);
	bubble(s,length);
	for(int i = 0;i < length;i++)
	printf("%d ",s[i]);
	printf("\n");
printf("------------------selection_sort-------------------\n");
	int ss[] = {12,51,62,11,99,24,19,9};
	selection(ss,length);
	for(int i = 0;i < length;i++)
	printf("%d ",ss[i]);
	printf("\n");
printf("------------------insertion_sort-------------------\n");
	int sss[] = {12,51,62,11,99,24,19,9};
	selection(sss,length);
	for(int i = 0;i < length;i++)
	printf("%d ",sss[i]);
	printf("\n");
printf("------------------quick_sort-------------------\n");
	int ssss[] = {12,51,62,11,99,24,19,9};
	quickSort(ssss,0,length-1);
	for(int i = 0;i < length;i++)
	printf("%d ",ssss[i]);
	printf("\n");
}

void bubble(int s[],intlength)
{	int flag = 0;
	int temp = 0;
	for(int i = 1;(i<length) && (!flag);i ++)
	{	flag = 1;
		for(int j = 0; j < (length-i);j ++)
			if(s[j] > s[j+1])
			{
				temp = s[j];
				s[j] = s[j+1];
				s[j+1] = temp;
				flag = 0;
			}
	}
} 
void selection(int s[],int length)// 從還未排序的選最小的做交換
{
	int temp = 0;
	for(int i = 0;i<length;i ++)
	{
		for(int j = i+1; j < length;j ++)
			if(s[i] > s[j])
			{
				temp = s[i];
				s[i] = s[j];
				s[j] = temp;
				
			}
	}
}
void insertion_sort(int arr[], int n) // 從還未排序中的最左邊的 插入已排序中順序
{
  for (int i = 0; i < n; i++) {
    int j = i;
    while (j > 0 && arr[j - 1] > arr[j]) {
      int temp = arr[j];
      arr[j] = arr[j - 1];
      arr[j - 1] = temp;
      j--;
    }
  }
}

void quickSort(int number[], int left, int right) //取一個軸心 分別向左和又排序
{ 
    if(left < right) 
    { 
        int i = left; 
        int j = right + 1; 

        while(1) 
        { 
            // 向右找
            while(i + 1 < 10 && number[++i] < number[left]) ;  
            // 向左找  
            while(j -1 > -1 && number[--j] > number[left]) ;  
            if(i >= j) 
                break; 
            SWAP(number[i], number[j]); 
        } 

        SWAP(number[left], number[j]); 

        quickSort(number, left, j-1);   // 對左邊進行遞迴 
        quickSort(number, j+1, right);  // 對右邊進行遞迴 
    } 
} 