#include<stdio.h>
int main(void)
{	
	for (int j = 0, i = 1; j < 10000; i += j)
		printf("%d %d ", i, j += i);
	return 0;
}

