#include "Board_SCCB.h"
#include "Board_Sensor.h"
#include "Board_I2C.h"
void Board_CameraReset(void)
{
	/* Do a power cycle */
	HAL_GPIO_WritePin(CAM_STBY_GPIO_Port,CAM_STBY_Pin,GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(CAM_STBY_GPIO_Port,CAM_STBY_Pin,GPIO_PIN_RESET);
	HAL_Delay(10);
	/* Reset the sensor */
	HAL_GPIO_WritePin(CAM_RST_GPIO_Port,CAM_RST_Pin,GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(CAM_RST_GPIO_Port,CAM_RST_Pin,GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(CAM_RST_GPIO_Port,CAM_RST_Pin,GPIO_PIN_SET);
	HAL_Delay(10);
}
uint8_t Board_SCCB_Probe(void)
{
	uint8_t reg = 0x00;
	uint8_t slv_addr = 0x00;

	for (int i=0; i<127; i++) {
		if (HAL_I2C_Master_Transmit(&BoardI2C, i, &reg, 1, 1000) == HAL_OK) {
			slv_addr = i;
			break;
		}
		if (i!=126) {
			HAL_Delay(1); // Necessary for OV7725 camera (not for OV2640).
		}
	}
	return slv_addr;
}

uint8_t Board_SCCB_Read(uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data)
{
	int ret = 0;
	if((HAL_I2C_Master_Transmit(&BoardI2C, slv_addr, &reg_addr, 1, 1000) != HAL_OK)
	|| (HAL_I2C_Master_Receive(&BoardI2C, slv_addr, reg_data, 1, 1000) != HAL_OK)) {
			ret = -1;
	}
	return ret;
}

uint8_t Board_SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data)
{
	uint8_t ret=0;
	uint8_t buf[] = {reg, data};
	if(HAL_I2C_Master_Transmit(&BoardI2C, slv_addr, buf, 2, 1000) != HAL_OK) {
			ret=0xFF;
	}
	return ret;
}

