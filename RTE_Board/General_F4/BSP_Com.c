#include "BSP_Com.h"
//--------------------------------------------------------------
// Definition aller UARTs
// Reihenfolge wie bei UART_NAME_t
//--------------------------------------------------------------
static BSP_COM_Handle_t ComControlArray[COM_N] = {
// Name, Clock               , AF-UART      ,UART  , Baud , Interrupt
  {
		COM_1,RCC_APB2Periph_USART1,GPIO_AF_USART1,USART1,115200,USART1_IRQn, // UART2 mit 115200 Baud
	// PORT , PIN      , Clock              , Source
		{GPIOA,GPIO_Pin_9,RCC_AHB1Periph_GPIOA,GPIO_PinSource9},  // TX an PA2
		{GPIOA,GPIO_Pin_10,RCC_AHB1Periph_GPIOA,GPIO_PinSource10},
	//BufferLen QUENELEN 
		32,64,
	}, // RX an PA3

// Name, Clock               , AF-UART      ,UART  , Baud, Interrupt
  {
		COM_3,RCC_APB1Periph_USART3,GPIO_AF_USART3,USART3,9600,USART3_IRQn, // UART3 mit 115200 Baud
	// PORT , PIN      , Clock              , Source
		{GPIOC,GPIO_Pin_10,RCC_AHB1Periph_GPIOC,GPIO_PinSource10},  // TX an PD8
		{GPIOC,GPIO_Pin_11,RCC_AHB1Periph_GPIOC,GPIO_PinSource11},
	//BufferLen QUENELEN 
		32,64,
	}, // RX an PD9
};
//--------------------------------------------------------------
// init aller UARTs
//--------------------------------------------------------------
static void COMTimer_Callback(void* arg);
static BSP_COM_Name_e ComTimerID;
void BSP_COM_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  BSP_COM_Name_e nr;
	
  for(nr=(BSP_COM_Name_e)0;nr<COM_N;nr++) {

    // Clock enable der TX und RX Pins
    RCC_AHB1PeriphClockCmd(ComControlArray[nr].TX.CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(ComControlArray[nr].RX.CLK, ENABLE);

    // Clock enable der UART
    if((ComControlArray[nr].UART==USART1) || (ComControlArray[nr].UART==USART6)) {
      RCC_APB2PeriphClockCmd(ComControlArray[nr].CLK, ENABLE);
    }
    else {
      RCC_APB1PeriphClockCmd(ComControlArray[nr].CLK, ENABLE);
    }

    // UART Alternative-Funktions mit den IO-Pins verbinden
    GPIO_PinAFConfig(ComControlArray[nr].TX.PORT,ComControlArray[nr].TX.SOURCE,ComControlArray[nr].AF);
    GPIO_PinAFConfig(ComControlArray[nr].RX.PORT,ComControlArray[nr].RX.SOURCE,ComControlArray[nr].AF);

    // UART als Alternative-Funktion mit PushPull
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    // TX-Pin
    GPIO_InitStructure.GPIO_Pin = ComControlArray[nr].TX.PIN;
    GPIO_Init(ComControlArray[nr].TX.PORT, &GPIO_InitStructure);
    // RX-Pin
    GPIO_InitStructure.GPIO_Pin =  ComControlArray[nr].RX.PIN;
    GPIO_Init(ComControlArray[nr].RX.PORT, &GPIO_InitStructure);

    // Oversampling
    USART_OverSampling8Cmd(ComControlArray[nr].UART, ENABLE);

    // init mit Baudrate, 8Databits, 1Stopbit, keine Paritaet, kein RTS+CTS
    USART_InitStructure.USART_BaudRate = ComControlArray[nr].BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(ComControlArray[nr].UART, &USART_InitStructure);

    // UART enable
    USART_Cmd(ComControlArray[nr].UART, ENABLE);

    // RX-Interrupt enable
    USART_ITConfig(ComControlArray[nr].UART, USART_IT_RXNE, ENABLE);

    // enable UART Interrupt-Vector
    NVIC_InitStructure.NVIC_IRQChannel = ComControlArray[nr].INT;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // RX-Puffer vorbereiten
		//申请内存
		ComControlArray[nr].ComBuffer.pu8Databuf = (uint8_t *)RTE_BGetz(MEM_RTE,ComControlArray[nr].DataBufferLen);
		ComControlArray[nr].ComBuffer.u16Datalength = 0;
		RTE_MessageQuene_Init(&ComControlArray[nr].ComBuffer.ComQuene,ComControlArray[nr].DataQueneLen);
  }
	//创建定时器
	RTE_RoundRobin_CreateTimer("COMTimer",20,0,0,COMTimer_Callback,&ComTimerID);
}
//--------------------------------------------------------------
// ein Byte per UART senden
//--------------------------------------------------------------
void BSP_COM_SendByte(BSP_COM_Name_e uart, uint16_t wert)
{
  // warten bis altes Byte gesendet wurde
  while (USART_GetFlagStatus(ComControlArray[uart].UART, USART_FLAG_TXE) == RESET);
  USART_SendData(ComControlArray[uart].UART, wert);
}

//--------------------------------------------------------------
// ein Daten Array per UART senden
// cnt = Anzahl der Daten die gesendet werden sollen
//--------------------------------------------------------------
void BSP_COM_SendArray(BSP_COM_Name_e uart, uint8_t *data, uint16_t cnt)
{
  uint32_t n;
  if(cnt==0) return;
  // sende alle Daten
  for(n=0;n<cnt;n++) {
    BSP_COM_SendByte(uart,data[n]);
  }  
}
static void COMTimer_Callback(void* arg)
{
	BSP_COM_Name_e* com_name=(BSP_COM_Name_e *)arg;
	if(ComControlArray[*com_name].ComBuffer.u16Datalength)
		RTE_MessageQuene_In(&ComControlArray[*com_name].ComBuffer.ComQuene,ComControlArray[*com_name].ComBuffer.pu8Databuf,
				ComControlArray[*com_name].ComBuffer.u16Datalength);
	memset(ComControlArray[*com_name].ComBuffer.pu8Databuf,0,ComControlArray[*com_name].DataBufferLen);
	ComControlArray[*com_name].ComBuffer.u16Datalength = 0;
}
static void BSP_COM_RecCallback(uint16_t byte,BSP_COM_Name_e com_name)
{
	ComTimerID = com_name;
	ComControlArray[com_name].ComBuffer.pu8Databuf[ComControlArray[com_name].ComBuffer.u16Datalength++] = byte;
	RTE_RoundRobin_ResetTimer("COMTimer");
}
BSP_COM_Data_t* BSP_COM_ReturnQue(BSP_COM_Name_e com_name)
{
	return &ComControlArray[com_name].ComBuffer;
}
//--------------------------------------------------------------
// UART1-Interrupt
//--------------------------------------------------------------
void USART1_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(USART1);
    // Byte speichern
    BSP_COM_RecCallback(wert,COM_1);
  }
}
//--------------------------------------------------------------
// UART3-Interrupt
//--------------------------------------------------------------
void USART3_IRQHandler(void) {
  uint16_t wert;

  if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
    // wenn ein Byte im Empfangspuffer steht
    wert=USART_ReceiveData(USART3);
    // Byte speichern
    BSP_COM_RecCallback(wert,COM_3);
  }
}

////--------------------------------------------------------------
//// UART2-Interrupt
////--------------------------------------------------------------
//void USART2_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(USART2);
//    // Byte speichern
//    P_UART_RX_INT(USART2_IRQn,wert);
//  }
//}



////--------------------------------------------------------------
//// UART4-Interrupt
////--------------------------------------------------------------
//void UART4_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(UART4);
//    // Byte speichern
//    P_UART_RX_INT(UART4_IRQn,wert);
//  }
//}

////--------------------------------------------------------------
//// UART5-Interrupt
////--------------------------------------------------------------
//void UART5_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(UART5);
//    // Byte speichern
//    P_UART_RX_INT(UART5_IRQn,wert);
//  }
//}

////--------------------------------------------------------------
//// UART6-Interrupt
////--------------------------------------------------------------
//void USART6_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(USART6, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(USART6);
//    // Byte speichern
//    P_UART_RX_INT(USART6_IRQn,wert);
//  }
//}
