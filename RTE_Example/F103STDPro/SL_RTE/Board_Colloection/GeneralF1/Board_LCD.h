#ifndef __BOARD_LCD_H
#define __BOARD_LCD_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
#include "APP_StateMachine.h"
#define LCD_COM COM_2
#define LCD_COMBUFLEN COM2_DATALEN
typedef enum {
  LCD_IDLE = 0x00,
	LCD_INIT = 0x01,
	LCD_WIFISELECT = 0x02,
	LCD_PASSWORDINPUT = 0x03,
	LCD_MAINPAGE = 0x04,
	LCD_FOODCONRROLCENTRE = 0x05,
	LCD_AIRCONRROLCENTRE = 0x06,
	LCD_TASKCNT
}LCD_RunTask_e;
typedef struct {
	bool FeedStatus;
	bool AirCleanerStatus;
	bool HumStatus;
	bool TemperStatus;
	bool ControlMode;
	uint8_t FeedTimePeriod;
	uint8_t SubPage;
	uint8_t PasswordKeyValue;
	uint8_t SSIDPassword[20];
	uint8_t TimerCnt[LCD_TASKCNT];
	bool *TimerIfRun[LCD_TASKCNT];
	uint32_t *TimerStartTick[LCD_TASKCNT];
	StateMachine_t LCDStateMachine;
}LCD_ControlHandle_t;
extern LCD_ControlHandle_t LCD_ControlHandle;
void LCD_TaskInit(void);
#endif
