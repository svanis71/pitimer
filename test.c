#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
  time_t tsec;
  struct tm *tid;
  time(&tsec);
  tid = localtime(&tsec);
  printf("Tiden är %02d:%02d DST=%d\n", tid->tm_hour, tid->tm_min, tid->tm_isdst);
  return 0;
}

