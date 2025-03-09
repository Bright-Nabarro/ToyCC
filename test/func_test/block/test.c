int test_assign()
{
	int a;
	{
		a = 42;
	}
	return a;
}

int test_block_local_var()
{
	int x = 0;
	{
		int x = 42;
		return x;
	}
	return x;
}

int test_block_nested()
{
    int x = 1;
    {
        int x = 2;
        {
            int x = 3;
            return x;  // 期望返回 3
        }
    }
    return x;  // 不会执行
}

int test_block_return()
{
    {
        return 100;  // 在 Block 内提前返回
    }
    return 200;  // 不会执行
}

