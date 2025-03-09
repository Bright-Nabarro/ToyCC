#include <stdio.h>
#include <stdlib.h>

int ret42();
int initial_ret42();
int value_ret42();
int initial2_ret42();

void report_error(int n, char* prg)
{
	if (n == 42)
	{
		printf("%s: return signed int 42 success\n", prg);
	}
	else
	{
		printf("%s: return signed int 42 failure, tested function returns %d\n", prg, n);
		exit(1);
	}
}

int main([[maybe_unused]] int argc, char* argv[])
{
	report_error(ret42(), argv[0]);
	report_error(initial_ret42(), argv[0]);
	report_error(value_ret42(), argv[0]);
	report_error(initial2_ret42(), argv[0]);
	return 0;
}
