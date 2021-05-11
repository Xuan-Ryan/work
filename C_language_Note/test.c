#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
void swap(int *address_a, int *address_b) {
    *address_a = *address_a ^ *address_b ; 
    *address_b = *address_b ^ *address_a;
    *address_a = *address_a ^ *address_b;
}

int main (void) {
	int a =1,b=0;

	swap(&a,&b);
	printf("%d\t%d\n",a,b );

	return 0;
}
