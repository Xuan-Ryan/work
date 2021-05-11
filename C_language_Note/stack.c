#include <stdio.h>
#include <stdlib.h>
#define MAXSTACK 100 /*定義最大堆疊容量*/

int stack[MAXSTACK];  //堆疊的陣列宣告 
int top = -1;		//堆疊的頂端
int isEmpty();
void push(int); 
int pop();
 
int main() 
{
	int value;
	int i;
	printf("請依序輸入10筆資料:\n");
	for(i = 0;i < 10;i ++){
		scanf("%d",&value);
		push(value);
	}
	printf("====================\n");
	while(!isEmpty()){
		printf("堆疊彈出的順序為:%d\n",pop()); 
	}
	pop();
	return 0;
}
/*判斷是否為空堆疊*/
int isEmpty()
{
	if(top == -1){
		return 1; 
	}else{
		return 0;
	}
} 
/*將指定的資料存入堆疊*/
void push(int data)
{
	if(top >= MAXSTACK){
		printf("堆疊已滿,無法再加入\n");	
	}
	else
	{
		top++;
		stack[top] = data;
	} 
 
} 
/*從堆疊取出資料*/
int pop()
{
	int data;
	if(isEmpty()){
		printf("堆疊已空\n");
		return 0; 
	}else{
		data = stack[top];
		top--;
		return data; 
	}
}