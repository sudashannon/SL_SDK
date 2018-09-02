#include  "BSP_ETH.h"
/*
*********************************************************************************************************
*	                                     变量
*********************************************************************************************************
*/
__IO uint8_t  g_ucEthLinkStatus = 0;  /* 以太网连接状态，0 表示未连接，1 表示连接 */
/* Net_Config.c */
extern uint8_t own_hw_adr[];
/* DMA描述符 */
tEMACDMADescriptor g_psTxDescriptor[NUM_TX_DESCRIPTORS];
tEMACDMADescriptor g_psRxDescriptor[NUM_RX_DESCRIPTORS];
uint8_t g_pui8TxBuffer[ETH_BUF_SIZE];
uint8_t g_pui8RxBuffer[ETH_BUF_SIZE];
uint32_t g_ui32TxDescIndex;
uint32_t g_ui32RxDescIndex;
static volatile uint32_t g_ui32Flags;
/**
 * Interrupt counters (for debug purposes).
 */
volatile uint32_t g_ui32NormalInts;
volatile uint32_t g_ui32AbnormalInts;
/*
*********************************************************************************************************
*	函 数 名: InitDMADescriptors
*	功能说明: MAC DMA接收描述符初始化。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitDescriptors(void)
{
	uint32_t ui32Loop;

	//
	// Initialize each of the transmit descriptors.  Note that we leave the OWN
	// bit clear here since we have not set up any transmissions yet.
	//
	for(ui32Loop = 0; ui32Loop < NUM_TX_DESCRIPTORS; ui32Loop++)
	{
			g_psTxDescriptor[ui32Loop].ui32Count =
					(DES1_TX_CTRL_SADDR_INSERT |
					 (ETH_BUF_SIZE << DES1_TX_CTRL_BUFF1_SIZE_S));
			g_psTxDescriptor[ui32Loop].pvBuffer1 = g_pui8TxBuffer;
			g_psTxDescriptor[ui32Loop].DES3.pLink =
					(ui32Loop == (NUM_TX_DESCRIPTORS - 1)) ?
					g_psTxDescriptor : &g_psTxDescriptor[ui32Loop + 1];
			g_psTxDescriptor[ui32Loop].ui32CtrlStatus =
					(DES0_TX_CTRL_LAST_SEG | DES0_TX_CTRL_FIRST_SEG |
					 DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_CHAINED |
					 DES0_TX_CTRL_IP_ALL_CKHSUMS);
	}

	//
	// Initialize each of the receive descriptors.  We clear the OWN bit here
	// to make sure that the receiver doesn't start writing anything
	// immediately.
	//
	for(ui32Loop = 0; ui32Loop < NUM_RX_DESCRIPTORS; ui32Loop++)
	{
			g_psRxDescriptor[ui32Loop].ui32CtrlStatus = 0;
			g_psRxDescriptor[ui32Loop].ui32Count =
					(DES1_RX_CTRL_CHAINED |
					 (ETH_BUF_SIZE << DES1_RX_CTRL_BUFF1_SIZE_S));
			g_psRxDescriptor[ui32Loop].pvBuffer1 = g_pui8RxBuffer;
			g_psRxDescriptor[ui32Loop].DES3.pLink =
					(ui32Loop == (NUM_RX_DESCRIPTORS - 1)) ?
					g_psRxDescriptor : &g_psRxDescriptor[ui32Loop + 1];
	}

	//
	// Set the descriptor pointers in the hardware.
	//
	EMACRxDMADescriptorListSet(EMAC0_BASE, g_psRxDescriptor);
	EMACTxDMADescriptorListSet(EMAC0_BASE, g_psTxDescriptor);

	//
	// Start from the beginning of both descriptor chains.  We actually set
	// the transmit descriptor index to the last descriptor in the chain
	// since it will be incremented before use and this means the first
	// transmission we perform will use the correct descriptor.
	//
	g_ui32RxDescIndex = 0;
	g_ui32TxDescIndex = NUM_TX_DESCRIPTORS - 1;
}
/*
*********************************************************************************************************
*	函 数 名: init_ethernet
*	功能说明: 初始化以太网RMII方式引脚，驱动PHY，配置MAC及其DMA方式。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void init_ethernet (void) 
{
	/* Enable Port F for Ethernet LEDs */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	// PF0/PF4 are used for Ethernet LEDs.
	GPIOPinConfigure(GPIO_PF0_EN0LED0);
	GPIOPinConfigure(GPIO_PF4_EN0LED1);
	GPIOPinTypeEthernetLED(GPIOF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
	// Enable the ethernet peripheral.
	//
	// Enable and reset the Ethernet modules.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);

	//
	// Wait for the MAC to be ready.
	//
	printf("[RL-Net]    等待MAC配置完成...\n");
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0));
  // Configure for use with the internal PHY.
	EMACPHYConfigSet(EMAC0_BASE, EMAC_PHY_CONFIG);
	printf("[RL-Net]    MAC配置成功!\n");
	// Reset the MAC.
	EMACReset(EMAC0_BASE);
	// Initialize the MAC and set the DMA mode.
	EMACInit(EMAC0_BASE, SystemCoreClock,
							 EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED, 4, 4,
							 0);
	// Set MAC configuration options.
	EMACConfigSet(EMAC0_BASE,
										(EMAC_CONFIG_FULL_DUPLEX | EMAC_CONFIG_CHECKSUM_OFFLOAD |
										 EMAC_CONFIG_7BYTE_PREAMBLE | EMAC_CONFIG_IF_GAP_96BITS |
										 EMAC_CONFIG_USE_MACADDR0 |
										 EMAC_CONFIG_SA_FROM_DESCRIPTOR |
										 EMAC_CONFIG_BO_LIMIT_1024),
										(EMAC_MODE_RX_STORE_FORWARD |
										 EMAC_MODE_TX_STORE_FORWARD |
										 EMAC_MODE_TX_THRESHOLD_64_BYTES |
										 EMAC_MODE_RX_THRESHOLD_64_BYTES), 0);

	printf("[RL-Net]    当前设备MAC地址:%x-%x-%x-%x-%x-%x\n",own_hw_adr[0],own_hw_adr[1],own_hw_adr[2],own_hw_adr[3],own_hw_adr[4],own_hw_adr[5]);
	// Program the hardware with its MAC address (for filtering).
	EMACAddrSet(EMAC0_BASE, 0, (uint8_t *)own_hw_adr);
	// Wait for the link to become active.
	printf("[RL-Net]    等待网络连接...\n");
	while((EMACPHYRead(EMAC0_BASE, 0, EPHY_BMSR) &
				 EPHY_BMSR_LINKSTAT) == 0);
	printf("[RL-Net]    网络连接建立!\n");
  uint16_t ui16Val;
 /* Clear any stray PHY interrupts that may be set. */
  ui16Val = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR1);
  ui16Val = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR2);

  /* Configure and enable the link status change interrupt in the PHY. */
  ui16Val = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_SCR);
  ui16Val |= (EPHY_SCR_INTEN_EXT | EPHY_SCR_INTOE_EXT);
  EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_SCR, ui16Val);
  EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR1, (EPHY_MISR1_LINKSTATEN |
               EPHY_MISR1_SPEEDEN | EPHY_MISR1_DUPLEXMEN | EPHY_MISR1_ANCEN));
  /* Read the PHY interrupt status to clear any stray events. */
  ui16Val = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR1);
  /**
   * Set MAC filtering options.  We receive all broadcast and mui32ticast
   * packets along with those addressed specifically for us.
   */
  EMACFrameFilterSet(EMAC0_BASE, (EMAC_FRMFILTER_HASH_AND_PERFECT |
                     EMAC_FRMFILTER_PASS_MULTICAST));
  /* Clear any pending MAC interrupts. */
  EMACIntClear(EMAC0_BASE, EMACIntStatus(EMAC0_BASE, false));
	/* Initialize the DMA descriptors. */
  InitDescriptors();
	// Enable the Ethernet MAC transmitter and receiver.
	EMACTxEnable(EMAC0_BASE);
	EMACRxEnable(EMAC0_BASE);
	NVIC_SetPriority(EMAC0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
	// Enable the Ethernet interrupt.
	IntEnable(INT_EMAC0);
	// Enable the Ethernet RX Packet interrupt source.
	EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE|EMAC_INT_TRANSMIT|EMAC_INT_PHY);
	// Mark the first receive descriptor as available to the DMA to start
	// the receive processing.
	g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus |= DES0_RX_CTRL_OWN;
}
/*
*********************************************************************************************************
*	函 数 名: int_enable_eth
*	功能说明: 使能以太网中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void int_enable_eth (void) 
{
  /* Enable the Ethernet interrupt. */
	IntEnable(INT_EMAC0);
}

/*
*********************************************************************************************************
*	函 数 名: int_disable_eth
*	功能说明: 使能以太网中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void int_disable_eth (void) 
{
  /* Disable the Ethernet interrupt. */
	IntDisable(INT_EMAC0);
}
/*
*********************************************************************************************************
*	函 数 名: send_frame
*	功能说明: 传递数据帧给MAC DMA发送描述符，并使能发送。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern osThreadId_t ThreadIDNET;
void send_frame (OS_FRAME *frame) 
{
	int_fast32_t i32Loop;
	int32_t i32BufLen = frame->length;
	HWREGBITW(&g_ui32Flags, FLAG_TXPKT) = 1;
	while(g_psTxDescriptor[g_ui32TxDescIndex].ui32CtrlStatus &
				DES0_TX_CTRL_OWN);
	if(i32BufLen > ETH_BUF_SIZE)
	{
			i32BufLen = ETH_BUF_SIZE;
	}
	// Copy the packet data into the transmit buffer.
	for(i32Loop = 0; i32Loop < i32BufLen; i32Loop++)
	{
			g_pui8TxBuffer[i32Loop] = *(&frame->data[0]+i32Loop);
	}
	// Move to the next descriptor.
	g_ui32TxDescIndex++;
	if(g_ui32TxDescIndex == NUM_TX_DESCRIPTORS)
	{
			g_ui32TxDescIndex = 0;
	}
	// Fill in the packet size and tell the transmitter to start work.
	g_psTxDescriptor[g_ui32TxDescIndex].ui32Count = (uint32_t)i32BufLen;
	g_psTxDescriptor[g_ui32TxDescIndex].ui32CtrlStatus =
			(DES0_TX_CTRL_LAST_SEG | DES0_TX_CTRL_FIRST_SEG | DES0_TX_CTRL_CHAINED | DES0_TX_CTRL_OWN);
	// Tell the DMA to reacquire the descriptor now that we've filled it in.
	EMACTxDMAPollDemand(EMAC0_BASE);
	osThreadFlagsSet(ThreadIDNET,0x0001U);
}
/**
 * Process interrupts from the PHY.
 *
 * should be called from the Stellaris Ethernet Interrupt Handler.  This
 * function will read packets from the Stellaris Ethernet fifo and place them
 * into a pbuf queue.  If the transmitter is idle and there is at least one packet
 * on the transmit queue, it will place it in the transmit fifo and start the
 * transmitter.
 *
 */
static void tivaif_process_phy_interrupt(void)
{
	uint16_t ui16Val, ui16Status;
	uint32_t ui32Config, ui32Mode, ui32RxMaxFrameSize;

	/* Read the PHY interrupt status.  This clears all interrupt sources.
	 * Note that we are only enabling sources in EPHY_MISR1 so we don't
	 * read EPHY_MISR2.
	 */
	ui16Val = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_MISR1);

	/* Read the current PHY status. */
	ui16Status = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_STS);

	/* Has the link status changed? */
	if(ui16Val & EPHY_MISR1_LINKSTAT)
	{
		/* Is link up or down now? */
		if(ui16Status & EPHY_STS_LINK)
		{
			/* Tell lwIP the link is up. */
			//printf("[RL-Net]The link is up.\n");
		}
		else
		{
			//printf("[RL-Net]The link is down.\n");
		}
	}
	/* Has the speed or duplex status changed? */
	if(ui16Val & (EPHY_MISR1_SPEED | EPHY_MISR1_SPEED | EPHY_MISR1_ANC))
	{
		/* Get the current MAC configuration. */
		EMACConfigGet(EMAC0_BASE, &ui32Config, &ui32Mode,
										&ui32RxMaxFrameSize);
		/* What speed is the interface running at now?
		 */
		if(ui16Status & EPHY_STS_SPEED)
		{
			/* 10Mbps is selected */
			ui32Config &= ~EMAC_CONFIG_100MBPS;
		}
		else
		{
			/* 100Mbps is selected */
			ui32Config |= EMAC_CONFIG_100MBPS;
		}

		/* Are we in fui32l- or half-duplex mode? */
		if(ui16Status & EPHY_STS_DUPLEX)
		{
			/* Fui32l duplex. */
			ui32Config |= EMAC_CONFIG_FULL_DUPLEX;
		}
		else
		{
			/* Half duplex. */
			ui32Config &= ~EMAC_CONFIG_FULL_DUPLEX;
		}

		/* Reconfigure the MAC */
		EMACConfigSet(EMAC0_BASE, ui32Config, ui32Mode, ui32RxMaxFrameSize);
	}
}
void ETH_Handler(void)
{
  uint32_t ui32Status;
	// Read and Clear the interrupt.
	ui32Status = EMACIntStatus(EMAC0_BASE, true);
	// If the interrupt really came from the Ethernet and not our
	// timer, clear it.
	if(ui32Status)
	{
		EMACIntClear(EMAC0_BASE, ui32Status);
	}
  /* Is this an interrupt from the PHY? */
  if(ui32Status & EMAC_INT_PHY)
  {
    tivaif_process_phy_interrupt();
  }
	// Check to see if an RX Interrupt has occurred.
	if(ui32Status & EMAC_INT_RECEIVE)
	{
    // By default, we assume we got a bad frame.
    uint32_t i32FrameLen = 0;
		OS_FRAME *frame;
    // Make sure that we own the receive descriptor.
    if(!(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus & DES0_RX_CTRL_OWN))
    {
			if(!(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus &
					 DES0_RX_STAT_ERR))
			{
					if(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus &
						 DES0_RX_STAT_LAST_DESC)
					{
							i32FrameLen =
									((g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus &
										DES0_RX_STAT_FRAME_LENGTH_M) >>
									 DES0_RX_STAT_FRAME_LENGTH_S);
							if(i32FrameLen > ETH_MTU)
							{
									i32FrameLen = ETH_MTU;
							}
						/* 申请动态内存，RxLen或上0x80000000表示动态内存不足了不会调用函数sys_error() */
							frame = alloc_mem (i32FrameLen | 0x80000000);
							for(uint32_t i32Loop = 0; i32Loop < i32FrameLen; i32Loop++)
							{
								*(&frame->data[0]+i32Loop) = g_pui8RxBuffer[i32Loop];
							}
							put_in_queue (frame);
					}
			}
			g_ui32RxDescIndex++;
			if(g_ui32RxDescIndex == NUM_RX_DESCRIPTORS)
			{
					g_ui32RxDescIndex = 0;
			}
			g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus = DES0_RX_CTRL_OWN;
			osThreadFlagsSet(ThreadIDNET,0x0001U);
    }
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
