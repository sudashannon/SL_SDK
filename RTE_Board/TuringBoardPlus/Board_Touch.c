#include "Board_Touch.h"
#include "Board_I2C.h"
#define  A1 	0xFFFFFF34
#define  A2 	0xFFFFFF7E
#define  B1 	0x000C7254
#define  B2 	0x0007A85A
#define TSC2007_ADDRESS        0x90
/*------------------------------------------------------------------------------------------------------------
  Function Name  : tsc2007_MeasureXpos
  Description    : Measure the X position of touch point.
  Input          : -Mode: Conversion mode.                    
  Output         : The X position of touch point.
  Return         : None
------------------------------------------------------------------------------------------------------------*/
static uint16_t tsc2007_MeasureXpos(void)
{
	uint8_t Xpos[2];
	uint16_t position;
	uint8_t command;
	command = Measure_X_Select | TSC2007_12_bit;
	HAL_I2C_Master_Transmit(&BoardI2C, TSC2007_ADDRESS, &command, 1,100);
	HAL_I2C_Master_Receive(&BoardI2C, TSC2007_ADDRESS, Xpos, 2, 100);
	position = ((Xpos[0]<<4) | (Xpos[1]>>4));
	return position;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : tsc2007_MeasureYpos
  Description    : Measure the Y position of touch point.
  Input          : -Mode: Conversion mode.                    
  Output         : The Y position of touch point.
  Return         : None
------------------------------------------------------------------------------------------------------------*/
static uint16_t tsc2007_MeasureYpos(void)
{
	uint8_t Ypos[2];
	uint16_t position;
	uint8_t command;
	command = Measure_Y_Select | TSC2007_12_bit;
	HAL_I2C_Master_Transmit(&BoardI2C, TSC2007_ADDRESS, &command, 1,100);
	HAL_I2C_Master_Receive(&BoardI2C, TSC2007_ADDRESS, Ypos, 2, 100);
	position = ((Ypos[0]<<4) | (Ypos[1]>>4));
	return position;
} 
/*------------------------------------------------------------------------------------------------------------
  Function Name  : tsc2007_MeasureZ1pos
  Description    : Measure the Z1 position of touch point.
  Input          : -Mode: Conversion mode.                    
  Output         : The Z1 position of touch point.
  Return         : None
------------------------------------------------------------------------------------------------------------*/
static uint16_t tsc2007_MeasureZ1pos(void)
{
	uint8_t Z1pos[2];
	uint16_t position;
	uint8_t command;
	command = Measure_Z1_Select | TSC2007_12_bit;
	HAL_I2C_Master_Transmit(&BoardI2C, TSC2007_ADDRESS, &command, 1,100);
	HAL_I2C_Master_Receive(&BoardI2C, TSC2007_ADDRESS, Z1pos, 2, 100);
	position = ((Z1pos[0]<<4) | (Z1pos[1]>>4));
	return position;
}

/*------------------------------------------------------------------------------------------------------------
  Function Name  : tsc2007_MeasureZ2pos
  Description    : Measure the Z2 position of touch point.
  Input          : -Mode: Conversion mode.                    
  Output         : The Z2 position of touch point.
  Return         : None
------------------------------------------------------------------------------------------------------------*/
static uint16_t tsc2007_MeasureZ2pos(void)
{
	uint8_t Z2pos[2];
	uint16_t position;
	uint8_t command;
	command = Measure_Z2_Select | TSC2007_12_bit;
	HAL_I2C_Master_Transmit(&BoardI2C, TSC2007_ADDRESS, &command, 1,100);
	HAL_I2C_Master_Receive(&BoardI2C, TSC2007_ADDRESS, Z2pos, 2, 100);
	position = ((Z2pos[0]<<4) | (Z2pos[1]>>4));
	return position;
}

/**
  * @brief  Return if there is touch detected or not.
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval Touch detected state.
  */
static uint8_t tsc2007_TS_DetectTouch()
{
  uint8_t ret = 0;
	volatile uint16_t z1_position = 0;
	z1_position = tsc2007_MeasureZ1pos();
	if(z1_position > 0x000F)
		ret = 1;
	return ret;
}
/**
  * @brief  Configures and enables the touch screen interrupts.
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t Board_Touch_ITConfig(void)
{
  return TS_OK;  
}

/**
  * @brief  Gets the touch screen interrupt status.
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t Board_Touch_ITGetStatus(void)
{
  /* Return the TS IT status */
  return TS_OK;
}
/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t Board_Touch_GetState(TS_StateTypeDef *TS_State)
{
	TS_State->TouchDetected = tsc2007_TS_DetectTouch();
  if(TS_State->TouchDetected)
  {
		TS_State->x = (((A1 * tsc2007_MeasureXpos()) + B1)/1000);
		TS_State->y = (((A2 * tsc2007_MeasureYpos()) + B2)/1000);
	}
  return TS_OK;
}

/**
  * @brief  Clears all touch screen interrupts.
  * @retval None
  */
void Board_Touch_ITClear(void)
{
	
}
#if GUI_USE_LVGL
bool Board_GUI_TouchScan(lv_indev_data_t *data)
{
	static int16_t last_x = 0;
	static int16_t last_y = 0;
	TS_StateTypeDef ts = {0};
	Board_Touch_GetState(&ts);
	if(ts.TouchDetected == 1)
	{
		data->point.x = (uint16_t)(ts.x);
		data->point.y = (uint16_t)(ts.y);
		last_x = data->point.x;
		last_y = data->point.y;
		data->state = LV_INDEV_STATE_PR;			
	}
	else
	{
		data->point.x = last_x;
		data->point.y = last_y;
		data->state = LV_INDEV_STATE_REL;
	}
	return false;
}
#endif
