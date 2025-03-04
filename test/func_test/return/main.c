#include <stdio.h>
int ret42();

int main([[maybe_unused]] int argc, char* argv[])
{
	int n = ret42();
	if (ret42() == 42)
	{
		printf("%s: return signed int 42 success\n", argv[0]);
		return 0;
	}
	else
	{
		printf("%s: return signed int 42 failure, tested function returns %d\n", argv[0], n);
		return 1;
	}
}
