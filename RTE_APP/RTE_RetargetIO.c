#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.7
*** History: 1.0 创建，移植自mdk的retargetio.c
             2.0 为RTE的升级做适配，这里没有添加不适用微库的代码，下次再说
*****************************************************************************/
#if RTE_USE_STDIO == 1 
extern int RTE_Putchar (int ch);
extern int RTE_Getchar (void);
#ifdef __MICROLIB
static int getchar_undo =  0;
static int getchar_ch   = -1;
/*************************************************
*** Function: 重写fputc
*************************************************/
__attribute__((weak))
int fputc (int c, FILE * stream) {
  (void)c;
  (void)stream;
  if (stream == &__stdout) {
    if (c == '\n') 
			RTE_Putchar('\r');
    return (RTE_Putchar(c));
  }
  return (-1);
}
/*************************************************
*** Function: 重写fgetc
*************************************************/
__attribute__((weak))
int fgetc (FILE * stream) {
  int ch;
  if (stream == &__stdin) {
    if (getchar_undo) {
      ch = getchar_ch;
      getchar_ch = -1;
      getchar_undo = 0;
      return (ch);
    }
    ch = RTE_Getchar();
    getchar_ch = ch;
    return (ch);
  }
  return (-1);
}
/*************************************************
*** Function: 重写__backspace
*************************************************/
__attribute__((weak))
int __backspace(FILE *stream);
int __backspace(FILE *stream) {
  if (stream == &__stdin) {
    if (getchar_ch != -1) {
      getchar_undo = 1;
      return (0);
    }
    return (-1);
  }
  return (-1);
}
/*************************************************
*** Function: 重写__aeabi_assert
*************************************************/
__attribute__((weak,noreturn))
void __aeabi_assert (const char *expr, const char *file, int line) {
  char str[12], *p;
  fputs("*** assertion failed: ", stderr);
  fputs(expr, stderr);
  fputs(", file ", stderr);
  fputs(file, stderr);
  fputs(", line ", stderr);
  p = str + sizeof(str);
  *--p = '\0';
  *--p = '\n';
  while (line > 0) {
    *--p = '0' + (line % 10);
    line /= 10;
  }
  fputs(p, stderr);
  abort();
}
__attribute__((weak))
void abort(void) {
  for (;;);
}
 
#else
#include <rt_sys.h>
#define RETARGET_SYS
/* IO device file handles. */
#define FH_STDIN    0x8001
#define FH_STDOUT   0x8002
#define FH_STDERR   0x8003
// User defined ...
/* Standard IO device name defines. */
const char __stdin_name[]  = ":STDIN";
const char __stdout_name[] = ":STDOUT";
const char __stderr_name[] = ":STDERR";
__attribute__((weak))
FILEHANDLE _sys_open (const char *name, int openmode) {
  (void)openmode;
 
  if (name == NULL) {
    return (-1);
  }
 
  if (name[0] == ':') {
    if (strcmp(name, ":STDIN") == 0) {
      return (FH_STDIN);
    }
    if (strcmp(name, ":STDOUT") == 0) {
      return (FH_STDOUT);
    }
    if (strcmp(name, ":STDERR") == 0) {
      return (FH_STDERR);
    }
    return (-1);
  }
  return (-1);
}
__attribute__((weak))
int _sys_close (FILEHANDLE fh) {
  switch (fh) {
    case FH_STDIN:
      return (0);
    case FH_STDOUT:
      return (0);
    case FH_STDERR:
      return (0);
  }
  return (-1);
}
__attribute__((weak))
int _sys_write (FILEHANDLE fh, const uint8_t *buf, uint32_t len, int mode) {
  int ch;
  (void)mode;
  switch (fh) {
    case FH_STDIN:
      return (-1);
    case FH_STDOUT:
      for (; len; len--) {
        ch = *buf++;
        if (ch == '\n') 
					RTE_Putchar('\r');
        RTE_Putchar(ch);
      }
      return (0);
    case FH_STDERR:
      return (0);
  }
  return (-1);
}
__attribute__((weak))
int _sys_read (FILEHANDLE fh, uint8_t *buf, uint32_t len, int mode) {
  int ch;
  (void)mode;
 
  switch (fh) {
    case FH_STDIN:
      ch = RTE_Getchar();
      if (ch < 0) {
        return ((int)(len | 0x80000000U));
      }
      *buf++ = (uint8_t)ch;
      RTE_Putchar(ch);
      len--;
      return ((int)(len));
    case FH_STDOUT:
      return (-1);
    case FH_STDERR:
      return (-1);
  }
  return (-1);
}
__attribute__((weak))
int _sys_istty (FILEHANDLE fh) {
 
  switch (fh) {
    case FH_STDIN:
      return (1);
    case FH_STDOUT:
      return (1);
    case FH_STDERR:
      return (1);
  }
 
  return (0);
}
__attribute__((weak))
int _sys_seek (FILEHANDLE fh, long pos) {
  (void)pos;
 
  switch (fh) {
    case FH_STDIN:
      return (-1);
    case FH_STDOUT:
      return (-1);
    case FH_STDERR:
      return (-1);
  }
  return (-1);
}
__attribute__((weak))
long _sys_flen (FILEHANDLE fh) {
  switch (fh) {
    case FH_STDIN:
      return (0);
    case FH_STDOUT:
      return (0);
    case FH_STDERR:
      return (0);
  }
  return (0);
}
#endif
#endif
