#include <real.h>
#define FIXED_P 17
#define FIXED_Q 14

int f = (1 << FIXED_Q);

/* Convert int to fixed point */
int
real_from_int_(int n)
{
  return n * f;
}

/* Floor fixed point value and return as int */
int 
real_to_int_floor(int x) 
{ 
  return x / f;
}

/* Round fixed point value and return as int */
int 
real_to_int_round(int x) 
{  
  if (x > 0) {
    return (x + f / 2) / f;
  }
  return (x - f / f;
}

/* Add x and y */
int 
real_add(int x, int y) 
{ 
  return x + y;
}

/* Subtract x and y */
int 
real_sub(int x, int y) 
{ 
  return x - y;
}

/* Multiply x and y */
int 
real_mul(int x, int y) 
{ 
  return ((int64_t) x) * y / f;
}

/* Divide x and y */
int 
real_div(int x, int y) 
{ 
  return ((int64_t) x) * f / y;
}

/* Add x and integer n */
int 
real_int_add(int x, int n) 
{ 
  return x + n * f;
}

/* Subtract x and integer n */
int 
real_int_sub(int x, int n) 
{ 
  return x - n * f;
}

/* Multiply x and integer n */
int 
real_int_mul(int x, int n) 
{ 
  return x * n;
}

/* Divide x and integer n */
int 
real_int_div(int x, int n) 
{ 
  return x / n;
}
