#ifndef JUDGE_LOG_H
#define JUDGE_LOG_H

#include <stdio.h>
#include <stdarg.h>

void log_init(const char *path, int enable_debug);
void log_debug(const char *format, ...);
void log_error(const char *format, ...);
void log_info(const char *format, ...);
void log_close(void);

#endif
