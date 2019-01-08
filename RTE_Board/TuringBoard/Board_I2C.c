#include "Board_I2C.h"
I2C_HandleTypeDef BoardI2C;
void Board_I2C_Init(void)
{
  BoardI2C.Instance = I2Cx;
  BoardI2C.Init.Timing = 0x10C0ECFF;
  BoardI2C.Init.OwnAddress1 = 0xFE;
  BoardI2C.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  BoardI2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  BoardI2C.Init.OwnAddress2 = 0xFE;
  BoardI2C.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  BoardI2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  BoardI2C.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&BoardI2C) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&BoardI2C, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&BoardI2C, 0) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
}
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  I2Cx_CLK_ENABLE(); 

  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
    
  /*##-3- Configure the NVIC for I2C ########################################*/   
  /* NVIC for I2Cx */
#if I2C_INTERRUPT_USE == 1
  HAL_NVIC_SetPriority(I2Cx_ER_IRQn, I2Cx_ER_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(I2Cx_ER_IRQn);
  HAL_NVIC_SetPriority(I2Cx_EV_IRQn, I2Cx_EV_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(I2Cx_EV_IRQn);
#endif
}
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
  
  /*##-1- Reset peripherals ##################################################*/
  I2Cx_FORCE_RESET();
  I2Cx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure I2C Tx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
  /* Configure I2C Rx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);
  
  /*##-3- Disable the NVIC for I2C ##########################################*/
#if I2C_INTERRUPT_USE == 1
  HAL_NVIC_DisableIRQ(I2Cx_ER_IRQn);
  HAL_NVIC_DisableIRQ(I2Cx_EV_IRQn);
#endif
}

#if I2C_INTERRUPT_USE == 1
void I2Cx_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&BoardI2C);
}
void I2Cx_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&BoardI2C);
}
#endif
