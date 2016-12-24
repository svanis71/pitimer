#ifndef LOGGING_H
#define LOGGING_H

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_DEBUG_LOW 2
#define LOG_LEVEL_DEBUG_HIGH 3

extern void jsvlog_debug(const char *fmt, ...);
extern void jsvlog_info(const char *fmt, ...);
extern void jsvlog_error(const char *fmt, ...);

extern int init_logging(const char *filename);
extern void close_log();

#endif
