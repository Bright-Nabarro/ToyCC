#include <stdio.h>
int ret42();

int main()
{
	int n = ret42();
	if (ret42() == 42)
	{
		printf("return signed int 42 success\n");
		return 0;
	}
	else
	{
		printf("return signed int 42 failure, tested function returns %d\n", n);
		return 1;
	}
}
