#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int test_assign();
int test_block_local_var();
int test_block_nested();
int test_block_return();

int main([[maybe_unused]] int argc, char* argv[])
{
#define REPORT_ERROR(func_name, expected)                                      \
	do                                                                         \
	{                                                                          \
		int n = func_name();                                                   \
		if (n == expected)                                                     \
		{                                                                      \
			printf("%s: %s matched\n", argv[0], #func_name);                   \
		}                                                                      \
		else                                                                   \
		{                                                                      \
			printf("%s: %s failure, expected: %d, actual: %d\n", argv[0],      \
				   #func_name, expected, n);                                   \
			exit(1);                                                           \
		}                                                                      \
	} while (0)

	REPORT_ERROR(test_assign, 42);
	REPORT_ERROR(test_block_local_var, 42);
	REPORT_ERROR(test_block_nested, 3);
	REPORT_ERROR(test_block_return, 100);
	
}

