#include "Board_SCCB.h"
#include "Board_I2C.h"
uint8_t Board_SCCB_Probe(void)
{
	for (uint8_t addr=0x08; addr<=0x77; addr++) {
		__disable_irq();
		if (HAL_I2C_IsDeviceReady(&BoardI2C, addr << 1, 10, HAL_MAX_DELAY) == HAL_OK) {
			__enable_irq();
			if((addr << 1)!=0x90 && (addr << 1)!=0xA0 && (addr << 1)!=0xA2)
			return(addr << 1);
		}
		__enable_irq();
	}
	return 0;
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

