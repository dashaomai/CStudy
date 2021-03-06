#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

#ifndef _RPC_LOG_H
#define _RPC_LOG_H

void console_log(const char *format, ...);
void console_err(const char *format, ...);

void _combie_date_and_write(FILE *fd, const char *format, va_list ap);
const char *_get_timestamp(void);

#if defined(DEBUG)
  #define LOG console_log
  #define ERR console_err
#else
  #define LOG console_log
  #define ERR console_err
#endif

#endif
