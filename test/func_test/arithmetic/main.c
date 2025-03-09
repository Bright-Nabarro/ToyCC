#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int int_add(int a, int b);
int int_sub(int a, int b);
int int_mul(int a, int b);
int int_div(int a, int b);
int test_var_const_add();
int test_var_const_sub();
int test_var_const_mul();
int test_var_const_div();
int test_var_const_mix1();
int test_var_const_mix2();

void report_if_err(bool flag, int x, int y, char op, int ret, char* prg_name)
{
	if (!flag)
	{
		printf ("%s: arithmetic[%c] error. x = %d, y = %d, result = %d\n", prg_name, op, x, y, ret);
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
			report_if_err(n == x + y, x, y, '+', n, argv[0]);
			n = int_sub(x, y);
			report_if_err(n == x - y, x, y, '-', n, argv[0]);
			n = int_mul(x, y);
			report_if_err(n == x * y, x, y, '*', n, argv[0]);
			if (y == 0)
				continue;
			n = int_div(x, y);
			report_if_err(n == x / y, x, y, '/', n, argv[0]);
		}
	}

#define CHECK_RESULT(prog, ret, expected)                                      \
	if ((ret) != (expected))                                                   \
	{                                                                          \
		printf("%s: arithmetic error. result = %d, expected = %d\n", prog,     \
			   ret, expected);                                                 \
		exit(1);                                                               \
	}

	CHECK_RESULT(argv[0], test_var_const_add(), 15);
	CHECK_RESULT(argv[0], test_var_const_sub(), 7);
	CHECK_RESULT(argv[0], test_var_const_mul(), 12);
	CHECK_RESULT(argv[0], test_var_const_div(), 4);
	CHECK_RESULT(argv[0], test_var_const_mix1(), 14);
	CHECK_RESULT(argv[0], test_var_const_mix2(), 9);

	printf("%s: success",argv[0]);
	return 0;
}
