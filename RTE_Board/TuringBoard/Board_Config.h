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
#if HSE_VALUE == 8000000
	//<o> PLL M分频参数
	//<i> 晶振/M即为输入PLL的时钟。
	#define PLL_M    		         (4U)
	//<o> PLL N倍频参数
	//<i> PLL输入时钟*N为下一阶段时钟。
	#define PLL_N    		         (400U)
#elif HSE_VALUE == 12000000
	//<o> PLL M分频参数
	//<i> 晶振/M即为输入PLL的时钟。
	#define PLL_M    		         (3U)
	//<o> PLL N倍频参数
	//<i> PLL输入时钟*N为下一阶段时钟。
	#define PLL_N    		         (200U)
#elif HSE_VALUE == 25000000
	//<o> PLL M分频参数
	//<i> 晶振/M即为输入PLL的时钟。
	#define PLL_M    		         (2U)
	//<o> PLL N倍频参数
	//<i> PLL输入时钟*N为下一阶段时钟。
	#define PLL_N    		         (64U)
#endif
//</h>
//=======================
//<h>Systick配置
//=======================
	//<o> Systick中断优先级
	//<i> 默认最低。
	#define SYSTICK_PreemptPriority    		 (0x15)
//</h>
//==================================================================
//<h>串口1有关配置
//==================================================================
	//<o> 串口1中断优先级
	#define USART1_PreemptPriority           0x03
	//<o> 串口1接收中断优先级
	#define USART1_DMARX_PreemptPriority     0x04
	//<e>使能DMA发送
	#define USART1_USE_DMA_TX                0
	#if USART1_USE_DMA_TX == 1
	//<o> 串口发送DMA中断优先级
	#define USART1_DMATX_PreemptPriority     0x03
	#endif
	//</e>
//</h>
//==================================================================
//<h>LCD有关配置
//==================================================================
	//==================================================================
	//<h>LCD SPI有关配置
	//==================================================================
	#define SPI4_NSS_Pin                     GPIO_PIN_3
	#define SPI4_NSS_GPIO_Port               GPIOE
	#define SPI4_PreemptPriority             0x05
	#define SPI4_DMATX_PreemptPriority       0x06
	//</h>
	#define LCD_DC_Pin                       GPIO_PIN_12
	#define LCD_DC_GPIO_Port                 GPIOE
	#define LCD_RST_Pin                      GPIO_PIN_15
	#define LCD_RST_GPIO_Port                GPIOE

	#define LCD_PEN_Pin                      GPIO_PIN_8
	#define LCD_PEN_GPIO_Port                GPIOE
	#define LCD_CS2_Pin                      GPIO_PIN_7
	#define LCD_CS2_GPIO_Port                GPIOE
	#define LCD_IO_IRQn                      EXTI9_5_IRQn        
	#define LCD_IO_IRQHandler                EXTI9_5_IRQHandler      
	#define LCD_IO_PreemptPriority           0x09
//</h>
//==================================================================
//<h>WIFI有关配置
//==================================================================
	#define WIFI_SPI_DEBUG                       0
	typedef enum
	{
		WIFI_NULL_EVENT = 0x00000000,
		WIFI_TCP_REC_EVENT = 0x00000001,
		WIFI_TCP_SED_EVENT = 0x00000002,
		WIFI_DRIVER_EVENT = 0x00000003,
	}WIFI_Event_e;
	#define WIFI_RESET_GPIO_PORT             		 GPIOB
	#define WIFI_RESET_PIN                   		 GPIO_PIN_5
	#define WIFI_RESET_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
	 
	#define WIFI_INT_GPIO_PORT             			 GPIOD
	#define WIFI_INT_PIN                   			 GPIO_PIN_4
	#define WIFI_INT_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOD_CLK_ENABLE()

	#define WIFI_CS_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOD_CLK_ENABLE()
	#define WIFI_CS_GPIO_PORT                    GPIOD
	#define WIFI_CS_PIN                          GPIO_PIN_6
//---------------------------------------------------------
// SPI时钟与选择
#define WIFI_SPI                             SPI1
#define WIFI_SPI_CLK_ENABLE()            __HAL_RCC_SPI1_CLK_ENABLE()
#define WIFI_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define WIFI_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define WIFI_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define WIFI_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define WIFI_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()
//---------------------------------------------------------
// SPI相关管脚配置
#define WIFI_SCK_PIN                     GPIO_PIN_3
#define WIFI_SCK_GPIO_PORT               GPIOB
#define WIFI_SCK_AF                      GPIO_AF5_SPI1
#define WIFI_MISO_PIN                    GPIO_PIN_4
#define WIFI_MISO_GPIO_PORT              GPIOB
#define WIFI_MISO_AF                     GPIO_AF5_SPI1
#define WIFI_MOSI_PIN                    GPIO_PIN_7
#define WIFI_MOSI_GPIO_PORT              GPIOD
#define WIFI_MOSI_AF                     GPIO_AF5_SPI1
//---------------------------------------------------------
// SPI中断配置
#define WIFI_SPI_PreemptPriority             0x07
#define WIFI_SPI_DMATX_PreemptPriority       0x08
#define WIFI_SPI_DMARX_PreemptPriority       0x08
#define WIFI_IO_INTERRUPT_USE  1
#if WIFI_IO_INTERRUPT_USE == 1
#define WIFI_IO_IRQn                     EXTI4_IRQn        
#define WIFI_IO_IRQHandler               EXTI4_IRQHandler      
#define WIFI_IO_PreemptPriority          0x07
#endif
//</h>
//==================================================================
//<h>SDCard有关配置
//==================================================================
#define SDMMCx                           SDMMC1
#define SDMMCx_CLK_ENABLE()              __HAL_RCC_SDMMC1_CLK_ENABLE()
#define SDMMCx_FORCE_RESET()             __HAL_RCC_SDMMC1_FORCE_RESET()
#define SDMMCx_RELEASE_RESET()           __HAL_RCC_SDMMC1_RELEASE_RESET()
#define SD_CD_Pin                        GPIO_PIN_13
#define SD_CD_GPIO_Port                  GPIOC
#define SD_CD_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOC_CLK_ENABLE()
#define SDMMCx_D0_Pin                    GPIO_PIN_8
#define SDMMCx_D0_GPIO_Port              GPIOC
#define SDMMCx_D0_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define SDMMCx_D0_AF                     GPIO_AF12_SDIO1
#define SDMMCx_D1_Pin                    GPIO_PIN_9
#define SDMMCx_D1_GPIO_Port              GPIOC
#define SDMMCx_D1_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define SDMMCx_D1_AF                     GPIO_AF12_SDIO1
#define SDMMCx_D2_Pin                    GPIO_PIN_10
#define SDMMCx_D2_GPIO_Port              GPIOC
#define SDMMCx_D2_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define SDMMCx_D2_AF                     GPIO_AF12_SDIO1
#define SDMMCx_D3_Pin                    GPIO_PIN_11
#define SDMMCx_D3_GPIO_Port              GPIOC
#define SDMMCx_D3_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define SDMMCx_D3_AF                     GPIO_AF12_SDIO1
#define SDMMCx_CK_Pin                    GPIO_PIN_12
#define SDMMCx_CK_GPIO_Port              GPIOC 
#define SDMMCx_CK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE() 
#define SDMMCx_CK_AF                     GPIO_AF12_SDIO1
#define SDMMCx_CMD_Pin                   GPIO_PIN_2
#define SDMMCx_CMD_GPIO_Port             GPIOD  
#define SDMMCx_CMD_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE() 
#define SDMMCx_CMD_AF                    GPIO_AF12_SDIO1

#define SDMMCx_INTERRUPT_USE              0
#if SDMMCx_INTERRUPT_USE == 1
#define SDMMCx_PreemptPriority           0x05
#define SDMMCx_IRQn                      SDMMC1_IRQn
#define SDMMCx_IRQHandler                SDMMC1_IRQHandler
#endif
//</h>
//==================================================================
//<h>CAMERA有关配置
//==================================================================
#define CAM_XCLK_FREQUENCY      (12000000)
//---------------------------------------------------------
//CAMERA 复位和STBY管脚
#define CAM_RST_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOD_CLK_ENABLE()
#define CAM_RST_Pin 									   GPIO_PIN_15
#define CAM_RST_GPIO_Port 						   GPIOD
#define CAM_STBY_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define CAM_STBY_Pin                     GPIO_PIN_12
#define CAM_STBY_GPIO_Port 						   GPIOA
//---------------------------------------------------------
//CAMERA SENSOR SCCB读取的设备ID
#define OV9650_ID       							   (0x96)
#define OV2640_ID       							   (0x26)
#define OV7725_ID       							   (0x77)
#define MT9V034_ID                       (0x13) 
#define LEPTON_ID                        (0x54)
//==================================================================
//DCMI 有关配置
//==================================================================
#define TIMx                           TIM1
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM1_CLK_ENABLE()
#define TIMx_PCLK_FREQ()               HAL_RCC_GetPCLK2Freq()
#define TIMx_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
#define TIMx_PIN                       (GPIO_PIN_8)
#define TIMx_PORT                      (GPIOA)
#define TIMx_AF                        (GPIO_AF1_TIM1)
#define DCMI_PreemptPriority           0x02
#define DCMI_VSYNC_IRQN                EXTI9_5_IRQn
#define DCMI_VSYNC_IRQ_LINE            (7)
//==================================================================
//I2C有关配置
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
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1
/* Definition for I2Cx's NVIC */
#define I2C_INTERRUPT_USE 0
#if I2C_INTERRUPT_USE == 1
#define I2Cx_EV_PreemptPriority         0x06
#define I2Cx_EV_IRQn                    I2C1_EV_IRQn
#define I2Cx_ER_IRQn                    I2C1_ER_IRQn
#define I2Cx_ER_PreemptPriority         0x07
#define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
#define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler
#endif
//</h>
#endif
/****************** (C) COPYRIGHT SuChow University Shannon********************/
