#ifndef _RPC_LOG_H
#define _RPC_LOG_H

void console_st_inited(void);

void console_log(const char *format, ...);
void console_err(const char *format, ...);

#if defined(DEBUG)
  #define LOG console_log
  #define ERR console_err
#else
  #define LOG console_log
  #define ERR console_err
#endif

#endif
