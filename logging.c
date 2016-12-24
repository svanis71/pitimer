#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <syslog.h>
#include "logging.h"
#include "mytypes.h"
#include "jsvtellstick.h"

#define MAXLOG 1048


void jsvlog_info(const char *fmt, ...)
{
  if(g_application.current_config.logg_config.level >= LOG_LEVEL_INFO) {
   struct tm info = now();
    fprintf(g_application.current_config.logg_config.fp, "I %04d-%02d-%02d %02d:%02d.%02d: ",
	    info.tm_year + 1900, info.tm_mon + 1, info.tm_mday,
	    info.tm_hour, info.tm_min, info.tm_sec);
    va_list args;
    va_start(args, fmt);
    vfprintf(g_application.current_config.logg_config.fp, fmt, args);
    va_end(args);
    fprintf(g_application.current_config.logg_config.fp, "\n");
    fflush(g_application.current_config.logg_config.fp);
  }  
}

void jsvlog_debug(const char *fmt, ...)
{
  if(g_application.current_config.logg_config.level >= LOG_LEVEL_DEBUG_LOW) {
    /*    pthread_mutex_lock(&g_application.mutex_lock);*/
    struct tm info = now();
    fprintf(g_application.current_config.logg_config.fp, "D %04d-%02d-%02d %02d:%02d.%02d: ",
	    info.tm_year + 1900, info.tm_mon + 1, info.tm_mday,
	    info.tm_hour, info.tm_min, info.tm_sec);
    va_list args;
    va_start(args, fmt);
    vfprintf(g_application.current_config.logg_config.fp, fmt, args);
    va_end(args);
    fprintf(g_application.current_config.logg_config.fp, "\n");
    fflush(g_application.current_config.logg_config.fp);
    /*    pthread_mutex_unlock(&g_application.mutex_lock);*/
  }
}

void jsvlog_error(const char *fmt, ...)
{
  char *msg = (char *)calloc(2048, sizeof(char));;
  
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);
  syslog(LOG_ERR, msg);
  
  /*  pthread_mutex_lock(&g_application.mutex_lock);*/
  struct tm info = now();
  fprintf(g_application.current_config.logg_config.fp, "E %04d-%02d-%02d %02d:%02d.%02d: ",
	  info.tm_year + 1900, info.tm_mon + 1, info.tm_mday,
	  info.tm_hour, info.tm_min, info.tm_sec);
  fprintf(g_application.current_config.logg_config.fp, msg);
  fprintf(g_application.current_config.logg_config.fp, "\n");
  fflush(g_application.current_config.logg_config.fp);
  /*  pthread_mutex_unlock(&g_application.mutex_lock);*/
}

/** public **/ int init_logging(const char *filename)
{
  g_application.current_config.logg_config.fp = fopen(g_application.current_config.logg_config.loggFile, "a");
  if(!g_application.current_config.logg_config.fp) {
    syslog(LOG_ERR, "Failed to create log file!");
    return 0;
  }
  return 1;
 }

/** public **/ void close_log() 
{
  if(g_application.current_config.logg_config.fp)
    fclose(g_application.current_config.logg_config.fp);  
}
