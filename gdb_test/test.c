/*************************************************************************
    > File Name: test.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Mon 04 Aug 2014 12:20:30 AM PDT
 ************************************************************************/

#include<stdio.h>
int func(int n)
{
	int sum = 0, i;
	for(i = 0; i < n; i++)
	{
		sum += i;
	}
	return sum;
}

void main()
{
	int i;
	int result = 0;

	for(i = 1; i <= 100; i++)
	{
		result +=i;
	}
	printf("result[1-100] = %d\n", result);
	printf("result[1-250] = %d\n", func(250));
}
