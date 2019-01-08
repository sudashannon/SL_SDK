/**
  ******************************************************************************
  * @file    Board_Config.h
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   硬件环境配置头文件，按照KEIL格式编写，为板子的硬件环境进行配置。
  * @version V1.0 2018/11/02 第一版
  ******************************************************************************
  */
#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//<s> BOARD_VERSION
//<i> 硬件板子版本号，具体定义见相关文档。
#define BOARD_VERSION "1.0"
//=======================
//<e> 纯净工程模板
//=======================
//<i> 板子是否使用纯净工程。
#define BOARD_TURING_PURE          1
 //</e>
//=======================
//<h>系统时钟配置
//=======================
	//<o> PLL M分频参数
	//<i> 晶振/M即为输入PLL的时钟。
	#define PLL_M    		         (5U)
	//<o> PLL N倍频参数
	//<i> PLL输入时钟*N为下一阶段时钟。
	#define PLL_N    		         (160U)
//</h>
//=======================
//<h>Systick配置
//=======================
	//<o> Systick中断优先级
	//<i> 默认最低。
	#define SYSTICK_PreemptPriority    		 (0x15)
//</h>
//=======================
//<h>SDRAM配置
//=======================
	//<o> SDRAM起始地址
	//<i> 默认为0，即没有，大小需要在Initial中手动修改。
	#define SDRAM_DEVICE_ADDR    		 	(0xD0000000)
	//<o> SDRAM大小
	//<i> 默认为0，即没有，Initial需要完成MPU的手动修改。
	#define SDRAM_DEVICE_SIZE    		 	(0x02000000)
	//<o> SDRAM使用的MDMA中断优先级
	//<i> 默认倒数第二。
	#define MDMA_PreemptPriority    		 (0x3)
	//SDRAM总线带宽
	#define SDRAM_MEMORY_WIDTH               FMC_SDRAM_MEM_BUS_WIDTH_32
	//SDRAM时钟
	#define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2
	#define REFRESH_COUNT                    ((uint32_t)0x0603)   /* SDRAM refresh counter (100Mhz FMC clock) */
	//SDRAM的MDMA配置
	#define SDRAM_MDMAx_CHANNEL               MDMA_Channel0
	#define SDRAM_MDMAx_IRQn                  MDMA_IRQn
//</h>
//=======================
//<h>DMA2D配置
//=======================
	//<o> DMA2D中断优先级
	//<i> 默认倒数第三。
	#define DMA2D_PreemptPriority    		 (0x2)
//</h>
//==================================================================
//<h>I2C有关配置
//==================================================================
/* Definition for I2Cx clock resources */
#define I2Cx                            I2C1
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()
/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1
/* Definition for I2Cx's NVIC */
#define I2C_INTERRUPT_USE 0
#if I2C_INTERRUPT_USE == 1
#define I2Cx_EV_PreemptPriority         0x12
#define I2Cx_EV_IRQn                    I2C1_EV_IRQn
#define I2Cx_ER_IRQn                    I2C1_ER_IRQn
#define I2Cx_ER_PreemptPriority         0x11
#define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
#define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler
#endif
//</h>
//==================================================================
//<h>LCD有关配置
//==================================================================
#define LCD_BL_Pin 				GPIO_PIN_10
#define LCD_BL_GPIO_Port 	GPIOF
//</h>
//==================================================================
//<h>串口1有关配置
//==================================================================
	//<o> 串口1中断优先级
	#define USART1_PreemptPriority           0x10
	//<o> 串口1接收中断优先级
	#define USART1_DMARX_PreemptPriority     0x09
	//<e>使能DMA发送
	#define USART1_USE_DMA_TX                0
	#if USART1_USE_DMA_TX == 1
	//<o> 串口发送DMA中断优先级
	#define USART1_DMATX_PreemptPriority     0x08
	#endif
	//</e>
//</h>
//==================================================================
//<h>SDCard有关配置
//==================================================================
#define SDMMCx                           SDMMC2
#define SDMMCx_CLK_ENABLE()              __HAL_RCC_SDMMC2_CLK_ENABLE()
#define SDMMCx_FORCE_RESET()             __HAL_RCC_SDMMC2_FORCE_RESET()
#define SDMMCx_RELEASE_RESET()           __HAL_RCC_SDMMC2_RELEASE_RESET()
#define SD_CD_Pin                        GPIO_PIN_11
#define SD_CD_GPIO_Port                  GPIOD
#define SD_CD_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOD_CLK_ENABLE()
#define SDMMCx_D0_Pin                    GPIO_PIN_14
#define SDMMCx_D0_GPIO_Port              GPIOB
#define SDMMCx_D0_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SDMMCx_D0_AF                     GPIO_AF9_SDIO2
#define SDMMCx_D1_Pin                    GPIO_PIN_15
#define SDMMCx_D1_GPIO_Port              GPIOB
#define SDMMCx_D1_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SDMMCx_D1_AF                     GPIO_AF9_SDIO2
#define SDMMCx_D2_Pin                    GPIO_PIN_3
#define SDMMCx_D2_GPIO_Port              GPIOB
#define SDMMCx_D2_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SDMMCx_D2_AF                     GPIO_AF9_SDIO2
#define SDMMCx_D3_Pin                    GPIO_PIN_4
#define SDMMCx_D3_GPIO_Port              GPIOB
#define SDMMCx_D3_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SDMMCx_D3_AF                     GPIO_AF9_SDIO2
#define SDMMCx_CK_Pin                    GPIO_PIN_6
#define SDMMCx_CK_GPIO_Port              GPIOD
#define SDMMCx_CK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE() 
#define SDMMCx_CK_AF                     GPIO_AF11_SDIO2
#define SDMMCx_CMD_Pin                   GPIO_PIN_0
#define SDMMCx_CMD_GPIO_Port             GPIOA 
#define SDMMCx_CMD_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE() 
#define SDMMCx_CMD_AF                    GPIO_AF9_SDIO2

#define SDMMCx_INTERRUPT_USE              0
#if SDMMCx_INTERRUPT_USE == 1
#define SDMMCx_PreemptPriority           0x07
#define SDMMCx_IRQn                      SDMMC2_IRQn
#define SDMMCx_IRQHandler                SDMMC2_IRQHandler
#endif
//</h>
//==================================================================
//<h>CAMERA有关配置
//==================================================================
//---------------------------------------------------------
//CAMERA 复位和STBY管脚
#define CAM_RST_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOI_CLK_ENABLE()
#define CAM_RST_Pin 									   GPIO_PIN_11
#define CAM_RST_GPIO_Port 						   GPIOI
#define CAM_STBY_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOE_CLK_ENABLE()
#define CAM_STBY_Pin                     GPIO_PIN_3
#define CAM_STBY_GPIO_Port 						   GPIOE
//---------------------------------------------------------
//CAMERA SENSOR SCCB读取的设备ID
#define OV9650_ID       							   (0x96)
#define OV2640_ID       							   (0x26)
#define OV7725_ID       							   (0x77)
#define MT9V034_ID                       (0x13) 
#define LEPTON_ID                        (0x54)
//---------------------------------------------------------
//CAMERA SENSOR SCCB时钟
#define CAM_XCLK_FREQUENCY             (12000000)
#define TIMx                           TIM1
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM1_CLK_ENABLE()
#define TIMx_PCLK_FREQ()               HAL_RCC_GetPCLK2Freq()
#define TIMx_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
#define TIMx_PIN                       (GPIO_PIN_8)
#define TIMx_PORT                      (GPIOA)
#define TIMx_AF                        (GPIO_AF1_TIM1)
//</h>
//==================================================================
//<h>DCMI 有关配置
//==================================================================
#define DCMI_PreemptPriority           0x04
//</h>
#endif
/****************** (C) COPYRIGHT SuChow University Shannon********************/
