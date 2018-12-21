#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strnlen(const char * s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

# define do_div(n, base) ({						\
	unsigned int __base = (base);					\
	unsigned int __rem;						\
	__rem = ((unsigned long long)(n)) % __base;			\
	(n) = ((unsigned long long)(n)) / __base;			\
	__rem;								\
})


static int skip_atoi(const char **s)
{
	int i, c;

	for (i = 0; '0' <= (c = **s) && c <= '9'; ++*s)
		i = i*10 + c - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

static char * number(char * str, unsigned long long num, int base, int size, int precision, int type)
{
	char c,sign,tmp[66];
	const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
	int i;

	if (type & LARGE)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if ((signed long long)num < 0) {
			sign = '-';
			num = - (signed long long)num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num != 0) {
		tmp[i++] = digits[do_div(num, base)];
	}
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}

int printf(const char *fmt, ...) {
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int len;
unsigned long long num;
int i, base;
char * str;
const char *s;

int flags;		/* flags to number() */

int field_width;	/* width of output field */
int precision;		/* min. # of digits for integers; max
         number of chars for from string */
int qualifier;		/* 'h', 'l', or 'L' for integer fields */
                        /* 'z' support added 23/7/1999 S.H.    */
      /* 'z' changed to 'Z' --davidm 1/25/99 */


for (str=out ; *fmt ; ++fmt) {
  if (*fmt != '%') {
    *str++ = *fmt;
    continue;
  }

  /* process flags */
  flags = 0;
  repeat:
    ++fmt;		/* this also skips first '%' */
    switch (*fmt) {
      case '-': flags |= LEFT; goto repeat;
      case '+': flags |= PLUS; goto repeat;
      case ' ': flags |= SPACE; goto repeat;
      case '#': flags |= SPECIAL; goto repeat;
      case '0': flags |= ZEROPAD; goto repeat;
      }

  /* get field width */
  field_width = -1;
  if ('0' <= *fmt && *fmt <= '9')
    field_width = skip_atoi(&fmt);
  else if (*fmt == '*') {
    ++fmt;
    /* it's the next argument */
    field_width = va_arg(ap, int);
    if (field_width < 0) {
      field_width = -field_width;
      flags |= LEFT;
    }
  }

  /* get the precision */
  precision = -1;
  if (*fmt == '.') {
    ++fmt;
    if ('0' <= *fmt && *fmt <= '9')
      precision = skip_atoi(&fmt);
    else if (*fmt == '*') {
      ++fmt;
      /* it's the next argument */
      precision = va_arg(ap, int);
    }
    if (precision < 0)
      precision = 0;
  }

  /* get the conversion qualifier */
  qualifier = -1;
  if (*fmt == 'l' && *(fmt + 1) == 'l') {
    qualifier = 'q';
    fmt += 2;
  } else if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L'
    || *fmt == 'Z') {
    qualifier = *fmt;
    ++fmt;
  }

  /* default base */
  base = 10;

  switch (*fmt) {
  case 'c':
    if (!(flags & LEFT))
      while (--field_width > 0)
        *str++ = ' ';
    *str++ = (unsigned char) va_arg(ap, int);
    while (--field_width > 0)
      *str++ = ' ';
    continue;

  case 's':
    s = va_arg(ap, char *);
    if (!s)
      s = "<NULL>";

    len = strnlen(s, precision);

    if (!(flags & LEFT))
      while (len < field_width--)
        *str++ = ' ';
    for (i = 0; i < len; ++i)
      *str++ = *s++;
    while (len < field_width--)
      *str++ = ' ';
    continue;

  case 'p':
    if (field_width == -1) {
      field_width = 2*sizeof(void *);
      flags |= ZEROPAD;
    }
    str = number(str,
      (unsigned long) va_arg(ap, void *), 16,
      field_width, precision, flags);
    continue;


  case 'n':
    if (qualifier == 'l') {
      long * ip = va_arg(ap, long *);
      *ip = (str - out);
    } else if (qualifier == 'Z') {
      size_t * ip = va_arg(ap, size_t *);
      *ip = (str - out);
    } else {
      int * ip = va_arg(ap, int *);
      *ip = (str - out);
    }
    continue;

  case '%':
    *str++ = '%';
    continue;

  /* integer number formats - set up the flags and "break" */
  case 'o':
    base = 8;
    break;

  case 'X':
    flags |= LARGE;
  case 'x':
    base = 16;
    break;

  case 'd':
  case 'i':
    flags |= SIGN;
  case 'u':
    break;

  default:
    *str++ = '%';
    if (*fmt)
      *str++ = *fmt;
    else
      --fmt;
    continue;
  }
  if (qualifier == 'l') {
    num = va_arg(ap, unsigned long);
    if (flags & SIGN)
      num = (signed long) num;
  } else if (qualifier == 'q') {
    num = va_arg(ap, unsigned long long);
    if (flags & SIGN)
      num = (signed long long) num;
  } else if (qualifier == 'Z') {
    num = va_arg(ap, size_t);
  } else if (qualifier == 'h') {
    num = (unsigned short) va_arg(ap, int);
    if (flags & SIGN)
      num = (signed short) num;
  } else {
    num = va_arg(ap, unsigned int);
    if (flags & SIGN)
      num = (signed int) num;
  }
  str = number(str, num, base, field_width, precision, flags);
}
*str = '\0';
return str-out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int i;

  va_start(ap, fmt);
  i = vsprintf(out,fmt,ap);
  va_end(ap);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
