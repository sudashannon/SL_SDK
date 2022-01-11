/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include <stdlib.h>

volatile unsigned char temp_A __attribute__((section (".ddr_ram")));

extern void sys_clock_init(void);
extern void sys_uart_init(void);
extern void sys_dram_init(void);
extern void sys_uart_putc(char c);

/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/
int main(void) {
  int i = 0;
  sys_uart_init();
  sys_clock_init();
  sys_dram_init();
  sys_uart_putc('A');
  sys_uart_putc('B');
  sys_uart_putc('C');
  temp_A = 'A';
  do {
    i++;
    temp_A++;
  } while (1);
}

/*************************** End of file ****************************/
