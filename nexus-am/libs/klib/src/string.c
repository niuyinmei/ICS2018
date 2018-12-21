#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    size_t i;
    for (i = 0; s[i] != '\0'; i++) ;
    return i;
}

char *strcpy(char* dst,const char* src) { //pass
	// 注意前后覆盖的问题
	return strncpy(dst, src, strlen(src));
}

char* strncpy(char* dst, const char* src, size_t n) { //pass
	size_t size_src = strlen(src);
	if( n > size_src) return strncpy(dst, src, size_src);
	else // n <= size_src
	{
		// printf("strncpy1 %s %s %d\n", dst, src, (int)n);
		// 注意前后覆盖的问题
		char ch[n+1];
		char *head = ch;
		size_t i = 0;
		while( i < n ) {
			*(head+i) = *(src+i);
			i++;
		}
		ch[n] = '\0';
		head = ch;
		char *result = dst;
		while((*(dst++) = *(head++))) ;
		// printf("strncpy2 %s\n", dst);
		return result;
	}
}

char* strcat(char* dst, const char* src) {
  size_t i,j;
  for (i = 0; dst[i] != '\0'; i++)
    ;
  for (j = 0; src[j] != '\0'; j++)
    dst[i+j] = src[j];
  dst[i+j] = '\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  while(*s1 && (*s1==*s2))
    s1++,s2++;
  return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  while(n--)
    if(*s1++!=*s2++)
      return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
  return 0;
}

void* memset(void* v,int c,size_t n) {
  unsigned char* p=v;
  while(n--)
    *p++ = (unsigned char)c;
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  char *dp = out;
  const char *sp = in;
  while (n--)
    *dp++ = *sp++;
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  const unsigned char *p1 = s1, *p2 = s2;
  while(n--)
    if( *p1 != *p2 )
      return *p1 - *p2;
    else
      p1++,p2++;
  return 0;
}

#endif
