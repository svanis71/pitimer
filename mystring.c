#include <stdlib.h>
#include <ctype.h>

#ifndef BYTE
typedef	unsigned char BYTE;
#endif


/* my string lib, just because I can :-) */

/***
	Removes trailing newline
***/
void chomp(char *str)
{
  for(int i = 0; *(str + i); i++) {
    if(*(str + i) == 10) {
      *(str + i) = '\0';
    }
  }
}

/***
	Converts string to uppercase
***/
void myupper(char *s)
{
	while((*s = toupper(*s)))
		s++;
}

/***
	Returns length of string
***/
int mystrlen(const char *s)
{
    int len = 0;
    while(*(s + len))
        len++;
    return len;
}

/***
	Copies string src into dest. Need to malloc dest before call.
***/
void mystrcpy(char *dest, const char *src)
{
    while( (*dest++ = *src++) );    
}

/***
    Concatenates two strings
***/
void mystrcat(char *dest, const char *src)
{
  while(*dest)dest++;
  mystrcpy(dest, src);
}

/***
	Compares strings. -1, 1 or 0. 0 is equal
***/
int mystrcmp(const char *s1, const char *s2)
{
    int ret = 0;
    int offset = 0;
    while(ret == 0 && *(s1 + offset) && *(s2 + offset))
    {
        if(*(s1 + offset) > *(s2 + offset)) ret = 1;
        if(*(s1 + offset) < *(s2 + offset)) ret = -1;
        offset++;
    }
    return ret;
}

/***
	Substring.
	src = Source string
	start = starting char. 0-indexed.
	len = length of substring. -1 means to the end of src.
	dest = destination.
***/
void mystrsub(const char *src, char *dest, int start, int len)
{
  int src_len = mystrlen(src);
  int di = 0;
  if(len < 0) {
    len = src_len - start;
  }
  for(int i = start; *(src + i) && i < (start + len); i++) {
    *(dest + di++) = *(src + i);
  }
  *(dest + di) = '\0';
}

/***
	Find index of character in a string.
	c = char to find
	str = string to search.
	
	returns zeroindexed position of c or -1 if not found.
***/
int indexof(char c, const char *str)
{
  int idx = -1;
  for(int i = 0; idx == -1 && i < mystrlen(str); i++) {
    if(*(str + i) == c) {
      idx = i;
    }
  }
  return idx;
}

/***
	Get first line from a multi line string. 
	
	maxlen = Max length to return.
***/
int get_line(char *dest, const char *src, int maxlen)
{
  int i = 0;
  int di = 0;
  while(i < maxlen && *(src + i) != 10 && *(src + i)) {
    *(dest + di++) = *(src + i);
    i++;
  }
  return i;
}

/***
	Sets sz bytes with value c in memory block p.
***/
void *mymemset(void *p, int c, size_t sz)
{
  BYTE *s = p;
  for(int i = 0; i < sz; i++)
    *s++ = (BYTE)c;
  return p;
}

void copy_characters(char *dest, const char *src, int stop)
{
	for(int i = 0, di = 0; i < stop && *(src + i); i++) {
		if(*(src +i) >= ' ') {
			*(dest + di++) = *(src + i);
		}
	}
}

int strequal(const char *s1, const char *s2)
{
    int i;
    if(mystrlen(s1) != mystrlen(s2))
      return 0;
    for(i = 0; *(s1 + i); i++)
        if(*(s1 + i) != *(s2 + i))
            return 0;
    return 1;
}

int strnequal(const char *s1, const char *s2, int n)
{
    int i;
    for(i = 0; i < n && *(s1 + i); i++)
      if(!(*(s2 + i)) || *(s1 + i) != *(s2 + i))
            return 0;
    return 1;
}

char *mystrsep(char **src, const char sep) 
{
    char *p1 = *src;
    for(; **src && **src != sep; (*src)++);
    *(*src)++ = '\0';
    return p1;
}

/** my string lib **/
