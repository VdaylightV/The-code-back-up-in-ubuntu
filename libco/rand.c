#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	int a;
	srand((unsigned int)time(0));//修改种子
	for (size_t i = 0; i < 10; i++)
		
	{	
		a = rand()%5+1;
		printf("%d  ", a);
		
	}
}
