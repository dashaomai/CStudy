#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

void console_log(const char *format, ...);
void console_err(const char *format, ...);

void _combie_date_and_write(FILE *fd, const char *format, va_list ap);
const char *_get_timestamp(void);

#if defined(DEBUG)
  #define LOG(format, ...) console_log(format, ...)
  #define ERR(format, ...) console_err(format, ...)
#else
  #define LOG(format, ...) void(0)
  #define ERR(format, ...) void(0)
#endif
