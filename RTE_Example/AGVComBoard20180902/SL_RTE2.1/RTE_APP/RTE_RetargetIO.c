#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.7
*** History: 1.0 创建，移植自mdk的retargetio.c
             2.0 为RTE的升级做适配，这里没有添加不适用微库的代码，下次再说
*****************************************************************************/
#if RTE_USE_HUMMANINTERFACE == 1 
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
#else
#pragma import(__use_no_semihosting_swi)

#include <rt_misc.h>
#include <rt_sys.h>
#include <time.h>
#define DEFAULT_HANDLE 0x100
/* Standard IO device handles. */
#define STDIN   0x8001
#define STDOUT  0x8002
#define STDERR  0x8003
/*
 * These names are special strings which will be recognized by 
 * _sys_open and will cause it to return the standard I/O handles, instead
 * of opening a real file.
 */
const char __stdin_name[] ="STDIN";
const char __stdout_name[]="STDOUT";
const char __stderr_name[]="STDERR";

/*
 * Open a file. May return -1 if the file failed to open. We do not require
 * this function to do anything. Simply return a dummy handle.
 */
FILEHANDLE _sys_open(const char * name, int openmode)
{
  /* Register standard Input Output devices. */
  if (strcmp(name, "STDIN") == 0) {
    return (STDIN);
  }
  if (strcmp(name, "STDOUT") == 0) {
    return (STDOUT);
  }
  if (strcmp(name, "STDERR") == 0) {
    return (STDERR);
  }
	return (DEFAULT_HANDLE);
}

/*
 * Close a file. Should return 0 on success or a negative value on error.
 * Not required in this implementation. Always return success.
 */
int _sys_close(FILEHANDLE fh)
{
    return 0; //return success
}

/*
 * Write to a file. Returns 0 on success, negative on error, and the number
 * of characters _not_ written on partial success. This implementation sends
 * a buffer of size 'len' to the UART.
 */
int _sys_write(FILEHANDLE fh, const unsigned char * buf,
               unsigned len, int mode)
{
  if (fh == STDOUT) {
    /* Standard Output device. */
    for (  ; len; len--) {
			if (*buf == '\n')  
			{
				RTE_Putchar('\r');
			}
			RTE_Putchar(*buf++);
    }
    return (0);
  }
  if (fh > 0x8000) {
    return (-1);
  }
	return (0);
}

/*
 * Read from a file. Can return:
 *  - zero if the read was completely successful
 *  - the number of bytes _not_ read, if the read was partially successful
 *  - the number of bytes not read, plus the top bit set (0x80000000), if
 *    the read was partially successful due to end of file
 *  - -1 if some error other than EOF occurred
 * This function receives a character from the UART, processes the character
 * if required (backspace) and then echo the character to the Terminal 
 * Emulator, printing the correct sequence after successive keystrokes.  
 */
int _sys_read(FILEHANDLE fh, unsigned char * buf,
              unsigned len, int mode)
{
     return 0;       
}

/*
 * Writes a character to the output channel. This function is used
 * for last-resort error message output.
 */
void _ttywrch(int ch)
{     
	if (ch == '\n')  
	{
		RTE_Putchar('\r');
	}
  RTE_Putchar(ch);
}

/*
 * Return non-zero if the argument file is connected to a terminal.
 */
int _sys_istty(FILEHANDLE fh)
{
  if (fh > 0x8000) {
    return (1);
  }
  return (0);
}

/*
 * Move the file position to a given offset from the file start.
 * Returns >=0 on success, <0 on failure. Seeking is not supported for the 
 * UART.
 */
int _sys_seek(FILEHANDLE fh, long pos)
{
    return -1; // error
}

/*
 * Flush any OS buffers associated with fh, ensuring that the file
 * is up to date on disk. Result is >=0 if OK, negative for an
 * error.
 */
int _sys_ensure(FILEHANDLE fh)
{
    return 0; // success
}

/*
 * Return the current length of a file, or <0 if an error occurred.
 * _sys_flen is allowed to reposition the file pointer (so Unix can
 * implement it with a single lseek, for example), since it is only
 * called when processing SEEK_END relative fseeks, and therefore a
 * call to _sys_flen is always followed by a call to _sys_seek.
 */
long _sys_flen(FILEHANDLE fh)
{
    return 0;
}

/*
 * Return the name for temporary file number sig in the buffer
 * name. Returns 0 on failure. maxlen is the maximum name length
 * allowed.
 */
int _sys_tmpnam(char * name, int sig, unsigned maxlen)
{
    return 0; // fail, not supported
}

/*
 * Terminate the program, passing a return code back to the user.
 * This function may not return.
 */
void _sys_exit(int returncode)
{
    while(1) {};
}

   /*
    * determines the current calendar time. The encoding of the value is
    * unspecified.
    * Returns: the implementations best approximation to the current calendar
    *          time. The value (time_t)-1 is returned if the calendar time is
    *          not available. If timer is not a null pointer, the return value
    *          is also assigned to the object it points to.*/

time_t time(time_t *p)
{
	return 0;
}
#endif
#endif
