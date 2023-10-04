#ifndef _FREESTDC_COMMON_MATHEX_H_
#define _FREESTDC_COMMON_MATHEX_H_

#define _PI_F 3.1415926f
#define _PI 3.141592653589793

#define _MIN(x, y) (((x) < (y)) ? (x) : (y))
#define _MAX(x, y) (((x) > (y)) ? (x) : (y))

int digcount(int x);
int getdigit(int x, int n);

int udigcount(unsigned int x);
int getudigit(unsigned int x, int n);

int lludigcount(unsigned long long x);
int getlludigit(unsigned long long x, long long n);

int xdigcount(unsigned int x);
int getxdigit(unsigned int x, int n);

int odigcount(unsigned int x);
int getodigit(unsigned int x, int n);

#endif
