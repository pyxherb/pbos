#include <common/mathex.h>
#include <math.h>
#include <stdint.h>

int digcount(int x)
{
	int count = 0;
	while (x)
		x /= 10, count++;
	return count;
}

int getdigit(int x, int n)
{
	if ((n > digcount(x)) || (n < 1))
		return -1;
	if (x < 0)
		x = -x;
	for (uint8_t i = 1; i < n; ++i)
		x /= 10;
	return x % 10;
}

int udigcount(unsigned int x)
{
	int count = 0;
	while (x)
		x /= 10, count++;
	return count;
}

int getudigit(unsigned int x, int n)
{
	if ((n > udigcount(x)) || (n < 1))
		return -1;
	for (uint8_t i = 1; i < n; ++i)
		x /= 10;
	return x % 10;
}

int lludigcount(unsigned long long x)
{
	int count = 0;
	while (x)
		x /= 10, count++;
	return count;
}

int getlludigit(unsigned long long x, long long n)
{
	if ((n > lludigcount(x)) || (n < 1))
		return -1;
	for (uint8_t i = 1; i < n; ++i)
		x /= 10;
	return x % 10;
}

int xdigcount(unsigned int x)
{
	int count = 0;
	while (x)
		x >>= 4, count++;
	return count;
}

int getxdigit(unsigned int x, int n)
{
	if ((n > xdigcount(x)) || (n < 1))
		return -1;
	for (uint8_t i = 1; i < n; ++i)
		x >>= 4;
	return x % 16;
}

int odigcount(unsigned int x)
{
	int count = 0;
	while (x)
		x >>= 3, count++;
	return count;
}

int getodigit(unsigned int x, int n)
{
	if ((n > odigcount(x)) || (n < 1))
		return -1;
	for (uint8_t i = 1; i < n; ++i)
		x >>= 3;
	return x % 8;
}
