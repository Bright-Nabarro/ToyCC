#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int int_add(int a, int b);
int int_sub(int a, int b);
int int_mul(int a, int b);
int int_div(int a, int b);

void report_if_err(bool flag, int x, int y, char op, int ret, char* prg_name)
{
	if (!flag)
	{
		printf ("%s: arithmetic[%c] error. x = %d, y = %d, result = %d", prg_name, op, x, y, ret);
		exit(1);
	}
}

int main([[maybe_unused]] int argc, char* argv[])
{
	int limits = 1000;
	for (int x = -limits; x < limits; ++x)
	{
		for (int y = -limits; y < limits; ++y)
		{
			int n = int_add(x, y);
			report_if_err(n != x + y, x, y, '+', n, argv[0]);
			n = int_sub(x, y);
			report_if_err(n != x - y, x, y, '-', n, argv[0]);
			n = int_mul(x, y);
			report_if_err(n != x * y, x, y, '*', n, argv[0]);
			n = int_div(x, y);
			report_if_err(n != x / y, x, y, '/', n, argv[0]);
		}
	}
	printf("%s: success",argv[0]);
	return 0;
}
