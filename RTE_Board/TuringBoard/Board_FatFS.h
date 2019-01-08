#ifndef __BOARD_FATFS_H
#define __BOARD_FATFS_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
#include "ff.h"
#include "ff_gen_drv.h"
extern char SDPath[4];   /* SD logical drive path */
extern FATFS SDFatFS;    /* File system object for SD logical drive */
extern FIL SDFile;       /* File object for SD */
void Board_FATFS_Init(void);
#endif
