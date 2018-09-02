#include "BSP_CAN.h"
BSP_CAN_Handle_t CANControlHandle[CAN_N]=
{
  // Name  ,                                                            
  {
		.CANName = CAN_1,
		.DataBufferLen = 8,
	}, 
};
void BSP_CAN_Init(BSP_CAN_NAME_e can_name)
{
	if(can_name == CAN_1)
	{
		//…Í«Î ˝æ›
		CANControlHandle[can_name].CANBuffer.pu8Databuf = (uint8_t *)RTE_BGetz(MEM_RTE,CANControlHandle[can_name].DataBufferLen);
		// GPIO
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		GPIOPinConfigure(GPIO_PB0_CAN1RX);
		GPIOPinConfigure(GPIO_PB1_CAN1TX);
		GPIOPinTypeCAN(GPIOB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
		// CAN
		SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN1);
		CANInit(CAN1_BASE);
		CANBitRateSet(CAN1_BASE, SystemCoreClock, 50000);
		CANIntEnable(CAN1_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);
		NVIC_SetPriority(CAN1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
		IntEnable(INT_CAN1);
		CANEnable(CAN1_BASE);
		CANControlHandle[can_name].CANMessage.ui32MsgID = 0;
		CANControlHandle[can_name].CANMessage.ui32MsgIDMask = 0;
		CANControlHandle[can_name].CANMessage.ui32Flags = (MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER);
		CANControlHandle[can_name].CANMessage.ui32MsgLen = 8;
		CANMessageSet(CAN1_BASE, 1, &CANControlHandle[can_name].CANMessage, MSG_OBJ_TYPE_RX);
	}
}
uint8_t BSP_CAN1_WriteData(uint16_t ID,uint8_t *data,uint8_t datalen)
{
  tCANMsgObject sCANMessage;
	sCANMessage.ui32MsgID = ID;
	sCANMessage.ui32MsgIDMask = 0;
	sCANMessage.ui32Flags = (MSG_OBJ_TX_INT_ENABLE);
	sCANMessage.ui32MsgLen = datalen;
	sCANMessage.pui8MsgData = data;	
	CANMessageSet(CAN1_BASE, 2, &sCANMessage, MSG_OBJ_TYPE_TX);
	return 0;
}
extern osThreadId_t ThreadIDCAN;
void CAN1_Handler(void)
{
    uint32_t ui32Status;
    // Read the CAN interrupt status to find the cause of the interrupt
    ui32Status = CANIntStatus(CAN1_BASE, CAN_INT_STS_CAUSE);
    // If the cause is a controller status interrupt, then get the status
    if(ui32Status == CAN_INT_INTID_STATUS)
    {
      ui32Status = CANStatusGet(CAN1_BASE, CAN_STS_CONTROL);
    }
    // Check if the cause is message object 1.
    else if(ui32Status == 1)
    {
			// Getting to this point means that the RX interrupt occurred on
			// message object 1, and the message reception is complete.  Clear the
			// message object interrupt.
			CANIntClear(CAN1_BASE, 1);
			CANControlHandle[CAN_1].CANMessage.pui8MsgData = CANControlHandle[CAN_1].CANBuffer.pu8Databuf;
			// Read the message from the CAN.  Message object number 1 is used
			// (which is not the same thing as CAN ID).  The interrupt clearing
			// flag is not set because this interrupt was already cleared in
			// the interrupt handler.
			CANMessageGet(CAN1_BASE, 1, &CANControlHandle[CAN_1].CANMessage, 0);
			IntDisable(INT_CAN1);
			osThreadFlagsSet(ThreadIDCAN,0x0001U);
    }
    // Check if the cause is message object 2.
    else if(ui32Status == 2)
    {
      CANIntClear(CAN1_BASE, 2);
    }
    // Check if the cause is message object 3.
    else if(ui32Status == 3)
    {
      CANIntClear(CAN1_BASE, 3);
    }
    // Otherwise, something unexpected caused the interrupt.  This should
    // never happen.
    else
    {

    }
}


