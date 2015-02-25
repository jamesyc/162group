#ifndef __LIB_REAL_H
#define __LIB_REAL_H

#include <stdint.h> // http://cs162.eecs.berkeley.edu/projects/resources/coding

int real_from_int_(int n);
int real_to_int_floor(int x);       // floor(x)
int real_to_int_round(int x);       // round(x)
int real_add(int x, int y);         // x + y
int real_sub(int x, int y);         // x - y
int real_mul(int x, int y);         // x * y / f
int real_div(int x, int y);         // x / y
int real_int_add(int x, int n);     // x + n*f
int real_int_sub(int x, int n);     // x - n*f
int real_int_mul(int x, int n);     // x * n
int real_int_div(int x, int n);     // x / n

#endif /* lib/real.h */