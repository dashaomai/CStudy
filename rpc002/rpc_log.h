#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

static void console_log(const char *format, ...);
static void console_err(const char *format, ...);

static void _combie_date_and_write(const int fd, const char *format, const va_list ap);
static const char *_get_timestamp(void);

#ifdef _DEBUG
  #define LOG(format, ...) console_log(format, ...)
  #define ERR(format, ...) console_err(format, ...)
#else
  #define LOG(format, ...) void(0)
  #define ERR(format, ...) void(0)
#endif
