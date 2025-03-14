#include "gcc_cpr.h"
#include <stdio.h>
#include <stdlib.h>

int test1(int n);
int test2(int n);

int main([[maybe_unused]] int argc, char* argv[])
{
#define REPORT_ERROR(func_name, x)                                             \
	do                                                                         \
	{                                                                          \
		int n = func_name(x);                                                  \
		int expected = gcc_##func_name(x);                                     \
		if (n != expected)                                                     \
			printf("%s: %s failure, expected: %d, actual: %d\n", argv[0],      \
				   #func_name, expected, n);                                   \
	} while (0)
	
	REPORT_ERROR(test1, 10);
	REPORT_ERROR(test2, 10);
}
