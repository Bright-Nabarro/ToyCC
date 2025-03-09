
int int_add(int a, int b)
{
	return a + b;
}

int int_sub(int a, int b)
{
	return a - b;
}

int int_mul(int a, int b)
{
	return a * b;
}

int int_div(int a, int b)
{
	return a / b;
}

int test_var_const_add()
{
    int x = 10;
    return x + 5;  // 10 + 5 = 15
}

int test_var_const_sub()
{
    int x = 10;
    return x - 3;  // 10 - 3 = 7
}

int test_var_const_mul()
{
    int x = 4;
    return x * 3;  // 4 * 3 = 12
}

int test_var_const_div()
{
    int x = 20;
    return x / 5;  // 20 / 5 = 4
}

int test_var_const_mix1()
{
    int x = 10;
    int y = x + 2 * 3 - 4 / 2;  // 10 + (2 * 3) - (4 / 2)
    return y;  // 10 + 6 - 2 = 14
}

int test_var_const_mix2()
{
    int x = 5;
    int y = 10;
    return x * 2 + y / 5 - 3;  // 5 * 2 + 10 / 5 - 3 = 10 + 2 - 3 = 9
}
