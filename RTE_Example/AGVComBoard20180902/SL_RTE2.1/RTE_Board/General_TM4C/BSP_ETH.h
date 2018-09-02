#ifndef __BSP_ETH_H
#define __BSP_ETH_H
#include "tm4c1294ncpdt.h"
#include "RTE_Include.h"
#include "hw_emac.h"
#include "hw_ints.h"
#include "hw_types.h"
#include "emac.h"
#include "sysctl.h"
#include "gpio.h"
#include "pin_map.h"
#include "interrupt.h"
#include "Net_Config.h"
#define NUM_RX_DESCRIPTORS 3
#define NUM_TX_DESCRIPTORS 3
#define ETH_BUF_SIZE       1536        /* ETH Receive/Transmit buffer size  */
//macÓëphyÅäÖÃ
#define EMAC_PHY_CONFIG (EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN |      \
                         EMAC_PHY_AN_100B_T_FULL_DUPLEX)
#define PHY_PHYS_ADDR      0
//*****************************************************************************
//
// A set of flags.  The flag bits are defined as follows:
//
//     0 -> An indicator that a SysTick interrupt has occurred.
//     1 -> An RX Packet has been received.
//     2 -> A TX packet DMA transfer is pending.
//     3 -> A RX packet DMA transfer is pending.
//
//*****************************************************************************
#define FLAG_SYSTICK            0
#define FLAG_RXPKT              1
#define FLAG_TXPKT              2
#define FLAG_RXPKTPEND          3
#endif
