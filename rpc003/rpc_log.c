#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <st.h>

#include "rpc_log.h"

static st_netfd_t STDOUT;
static st_netfd_t STDERR;

void _combie_date_and_write(int fd, const char *format, va_list ap);
const char *_get_timestamp(void);

const char *_get_timestamp(void) {
  static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  static char result[32];
  static time_t lastt = 0;

  struct tm *ptm;
  time_t currt = st_time();

  if (currt == lastt)
    return result;

  ptm = localtime(&currt);
  snprintf(result, sizeof(result), "[%02d/%s/%d %02d:%02d:%02d] ", ptm->tm_mday,
           months[ptm->tm_mon], 1900 + ptm->tm_year, ptm->tm_hour,
           ptm->tm_min, ptm->tm_sec);
  lastt = currt;

  return result;
}

void console_log(const char *format, ...) {
  va_list ap;

  va_start(ap, format);
  _combie_date_and_write(STDOUT_FILENO, format, ap);
  va_end(ap);
}

void console_err(const char *format, ...) {
  va_list ap;

  va_start(ap, format);
  _combie_date_and_write(STDERR_FILENO, format, ap);
  va_end(ap);
}

void _combie_date_and_write(int fd, const char *format, va_list ap) {
  char message[512];

  strcpy(message, _get_timestamp());
  vsprintf(message + strlen(message), format, ap);

  switch (fd) {
    case STDOUT_FILENO:
      if (STDOUT) {
        st_write(STDOUT, message, strlen(message), ST_UTIME_NO_TIMEOUT);
      } else {
        write(fd, message, strlen(message));
      }

      break;

    case STDERR_FILENO:
      if (STDERR) {
        st_write(STDERR, message, strlen(message), ST_UTIME_NO_TIMEOUT);
      } else {
        write(fd, message, strlen(message));
      }

      break;
  }
}


void console_st_inited(void) {
  STDOUT = st_netfd_open(STDOUT_FILENO);
  STDERR = st_netfd_open(STDERR_FILENO);
}
