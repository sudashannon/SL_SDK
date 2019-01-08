#ifndef __BOARD_WIFI_H
#define __BOARD_WIFI_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
#include "RAK_Include.h"
#include "WIFI_BSDSocket.h"
#include "WIFI_APSTA.h"
#include "WIFI_TCPUDP.h"
extern RW_APP_CTX        WIFI_CTX;
extern rw_WlanConnect_t  WIFI_ConnectInfo;
extern rw_WlanEasyConfigWpsResponse_t       WIFI_EasyWPSInfo;
extern rw_DriverParams_t WIFIDriver;
extern rw_IpConfig_t WIFI_APIP;
extern int Board_WIFI_Init(void);
#endif
