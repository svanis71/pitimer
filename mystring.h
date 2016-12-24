#ifndef MYSTRING_H
#define MYSTRING_H

#include <ctype.h>

extern int indexof(char c, const char *str);
extern void chomp(char *str);
extern void myupper(char *s);
extern int mystrlen(const char *s);
extern void mystrcpy(char *dest, const char *src);
extern void mystrcat(char *dest, const char *src);
extern int mystrcmp(const char *s1, const char *s2);
extern void mystrsub(const char *src, char *dest, int start, int len);
extern int get_line(char *dest, const char *src, int maxlen);
extern void *mymemset(void *p, int c, size_t sz);
extern void copy_characters(char *dest, const char *src, int stop);
extern int strequal(const char *s1, const char *s2);
extern int strnequal(const char *s1, const char *s2, int n);
extern char *mystrsep(char **src, const char sep);

#endif
