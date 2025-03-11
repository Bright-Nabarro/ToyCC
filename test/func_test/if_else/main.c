#include <stdio.h>
#include <stdlib.h>

int test_if_else(int x);
int test_if_else_no_braces(int x);
int test_if(int x);
int test_if_no_braces(int x);
int test_multi_branch(int x);
int test_multi_no_branch(int x);
int test_modify_var(int x);
int test_dangling_else(int x, int y);
// compile with gcc
int gcc_test_if_else(int x)
{
	if (x > 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

int gcc_test_if_else_no_braces(int x)
{
	if (x > 0)
		return 1;
	else
		return -1;
}

int gcc_test_if(int x)
{
	if (x > 0)
		return 1;
	return -1;
}

int gcc_test_if_no_braces(int x)
{
	if (x > 0) // ❌ 没有 `{}`，只会作用于下一行
		return 1;
	return 0; // 这行**不受 if 影响**
}

int gcc_test_multi_branch(int x)
{
	if (x < 0)
	{
		return -1;
	}
	else if (x == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int gcc_test_multi_no_branch(int x)
{
	if (x < 0)
		return -1;
	else if (x == 0)
		return 0;
	else 
		return 1;
}

int gcc_test_modify_var(int x)
{
	int y = 10;
	if (x > 0)
	{
		y = y + 5;
	}
	else
	{
		y = y - 5;
	}
	return y;
}

int gcc_test_dangling_else(int x, int y) {
    if (x > 0)
        if (y > 0)
            return 1;
    else  // ❌ 这个 else 可能匹配 `if(y > 0)` 而不是 `if(x > 0)`
        return -1;
    return 0;
}



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

#define REPORT_ERROR2(func_name, x, y)                                         \
	do                                                                         \
	{                                                                          \
		int n = func_name(x, y);                                               \
		int expected = gcc_##func_name(x, y);                                  \
		if (n != expected)                                                     \
			printf("%s: %s failure, expected: %d, actual: %d\n", argv[0],      \
				   #func_name, expected, n);                                   \
	} while (0)

	for (int x = -1000; x < 1001; ++x)
	{
		REPORT_ERROR(test_if_else, x);
		REPORT_ERROR(test_if_else_no_braces, x);
		REPORT_ERROR(test_if, x);
		REPORT_ERROR(test_if_no_braces, x);
		REPORT_ERROR(test_multi_branch, x);
		REPORT_ERROR(test_multi_no_branch, x);
		REPORT_ERROR(test_modify_var, x);
		for (int y = -10; y < 11; ++y)
			REPORT_ERROR2(test_dangling_else, x, y);
	}
}
