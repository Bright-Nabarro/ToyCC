int test1(int n)
{
	int result = 0;
	while(n > 0)
	{
		result = result + 1;
		n = n - 1;
	}
	return result;
}

int test2(int n)
{
	int result = 0;
	int n2 = 0;
	while(n > 0)
	{
		n2 = n;
		while(n2 > 0)
		{
			++result;
			n2 = n2 - 1;
		}
		n = n -1;

	}
	return result;
}
