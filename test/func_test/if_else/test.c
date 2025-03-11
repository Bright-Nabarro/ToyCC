
int test_if_else(int x)
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

int test_if_else_no_braces(int x)
{
	if (x > 0)
		return 1;
	else
		return -1;
}

int test_if(int x)
{
	if (x > 0)
		return 1;
	return -1;
}

int test_if_no_braces(int x)
{
	if (x > 0) // ❌ 没有 `{}`，只会作用于下一行
		return 1;
	return 0; // 这行**不受 if 影响**
}

int test_multi_branch(int x)
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

int test_multi_no_branch(int x)
{
	if (x < 0)
		return -1;
	else if (x == 0)
		return 0;
	else 
		return 1;
}

int test_modify_var(int x)
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

int test_dangling_else(int x, int y) {
    if (x > 0)
        if (y > 0)
            return 1;
    else  // ❌ 这个 else 可能匹配 `if(y > 0)` 而不是 `if(x > 0)`
        return -1;
    return 0;
}

