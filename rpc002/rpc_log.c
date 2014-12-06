#include "rpc_log.h"

static const char *_get_timestamp(void) {
  static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  static char result[32];
  static time_t lastt = 0;

  struct tm *ptm;
  time_t currt = st_time();

  if (currt == lastt)
    return result;

  ptm = localtime(&currt);
  snprintf(result, sizeof(result), "[%02d/%s/%d:%02d:%02d:%02d] ", ptm->tm_mday,
           months[ptm->tm_mon], 1900 + ptm->tm_year, ptm->tm_hour,
           ptm->tm_min, ptm->tm_sec);
  lastt = currt;

  return result;
}

static void console_log(const char *format, ...) {
  const va_list ap;

  va_start(ap, format);
  _combie_date_and_write(stdout, format, ap);
  va_stop(ap);
}

static void console_err(const char *format, ...) {
  const va_list ap;

  va_start(ap, format);
  _combie_date_and_write(stderr, format, ap);
  va_stop(ap);
}

static void _combie_date_and_write(const int fd, const char *format, const va_list ap) {
  char message[512];

  strcpy(message, _get_timestamp());
  vsprintf(message + strlen(message), format, ap);

  write(fd, message, strlen(message));
}
