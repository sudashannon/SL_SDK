#ifndef __BOARD_ESP8266_H
#define __BOARD_ESP8266_H
#include "APP_Include.h"
#include "BSP_Include.h"
#include "Board_ESP8266_Typedefs.h"
#define ESP8266_COM    COM_3
#define ESP8266_BUFLEN 12*COM_BUFFER_SIZE
typedef enum
{
	SEND_OK = 0x00,
	CONNECTSERVER_FAIL = 0x01,
	CIPSEND_FAIL = 0x02,
	DATASEND_FAIL = 0x03,
	RECRESPOND_FAIL = 0x04,
	DISCONNECTSERVER_FAIL = 0x05,
	SERVERMODEEXIT_FAIL = 0x06,
	SINGLECONNECTMODE_FAIL = 0x07,
	SERVERMODEENTER_FAIL = 0x08,
}ESP8266_SendState_e;
typedef struct
{
	uint8_t SelectSSID;
	uint8_t SSIDPassword[16];
	uint8_t **SSIDName;
}ESP8266_SSIDInfor_t;
typedef struct
{
	bool ModuleReady;
	bool ServerSet;
	bool ClientConnect;
	bool WIFIConnect;
	uint8_t *ESP8266RecBuf;
	uint8_t WifiCnt;
	uint8_t IP[4];
	uint8_t ConnectClientNum;
	uint32_t DataUploadTick;
	ESP8266_SSIDInfor_t SSIDInfor;
}ESP8266_WifiInfor_t;
typedef struct
{
	uint8_t Temper[5];
	uint8_t Weather[20];
	uint8_t Hour;
	uint8_t Min;
	uint8_t Second;
	uint8_t Date[20];
}ESP8266_Weather_t;
extern ESP8266_WifiInfor_t WifiInfor;
extern ESP8266_Weather_t ESP8266_Weather;
bool ATComand_Send(char *SendStr,const char *ExpectRec,uint32_t TimeOut);
bool ESP8266WifiConnect(uint8_t *SSID,uint8_t *Password);
void ESP8266Reset(void);
void ESP8266Init(void);
ESP8266_SendState_e ESP8266SendWeatherGet(const char* City);
ESP8266_SendState_e ESP8266SendTimeGet(void);
void ESP8266WiFiScan(void);
void ESP8266WiFiScanRelease(void);
void ESP8266IPGet(void);
void ESP8266TCPServerPoll(void);
bool ESP8266SetTCPServer(void);
bool ESP8266DisconnectTCPServer(void);


#define ESP_COM    COM_3
#define ESP_BUFLEN 2048
#define ESP_MAX_WAIT 0xFFFF
typedef enum {
	ESP_CMD_IDLE = 0,                           /*!< IDLE mode */
	/*
	 * Basic AT commands
	 */
	ESP_CMD_RESET,                              /*!< Reset device */
	ESP_CMD_ATE0,                               /*!< Disable ECHO mode on AT commands */
	ESP_CMD_ATE1,                               /*!< Enable ECHO mode on AT commands */
	ESP_CMD_GMR,                                /*!< Get AT commands version */
	ESP_CMD_GSLP,                               /*!< Set ESP to sleep mode */
	ESP_CMD_RESTORE,                            /*!< Restore ESP internal settings to default values */
	ESP_CMD_UART,
	ESP_CMD_SLEEP,
	ESP_CMD_WAKEUPGPIO,
	ESP_CMD_RFPOWER,
	ESP_CMD_RFVDD,
	ESP_CMD_RFAUTOTRACE,
	ESP_CMD_SYSRAM,
	ESP_CMD_SYSADC,
	ESP_CMD_SYSIOSETCFG,
	ESP_CMD_SYSIOGETCFG,
	ESP_CMD_SYSGPIODIR,
	ESP_CMD_SYSGPIOWRITE,
	ESP_CMD_SYSGPIOREAD,
	ESP_CMD_SYSMSG,                             /*!< Configure system messages */
	ESP_CMD_SYSMSG_CUR,
	/*
	 * WiFi based commands
	 */
	ESP_CMD_WIFI_CWMODE,                        /*!< Set/Get wifi mode */
	ESP_CMD_WIFI_CWLAPOPT,                      /*!< Configure what is visible on CWLAP response */
	ESP_CMD_WIFI_CWJAP,                         /*!< Connect to access point */
	ESP_CMD_WIFI_CWQAP,                         /*!< Disconnect from access point */
	ESP_CMD_WIFI_CWLAP,                         /*!< List available access points */
	ESP_CMD_WIFI_CIPSTAMAC_GET,                 /*!< Get MAC address of ESP station */
	ESP_CMD_WIFI_CIPSTAMAC_SET,                 /*!< Set MAC address of ESP station */
	ESP_CMD_WIFI_CIPSTA_GET,                    /*!< Get IP address of ESP station */
	ESP_CMD_WIFI_CIPSTA_SET,                    /*!< Set IP address of ESP station */
	ESP_CMD_WIFI_CWAUTOCONN,                    /*!< Configure auto connection to access point */
	ESP_CMD_WIFI_CWSAP_GET,                     /*!< Get software access point configuration */
	ESP_CMD_WIFI_CWSAP_SET,                     /*!< Set software access point configuration */
	ESP_CMD_WIFI_CIPAPMAC_GET,                  /*!< Get MAC address of ESP access point */
	ESP_CMD_WIFI_CIPAPMAC_SET,                  /*!< Set MAC address of ESP access point */
	ESP_CMD_WIFI_CIPAP_GET,                     /*!< Get IP address of ESP access point */
	ESP_CMD_WIFI_CIPAP_SET,                     /*!< Set IP address of ESP access point */
	ESP_CMD_WIFI_CWLIF,                         /*!< Get connected stations on access point */
	ESP_CMD_WIFI_WPS,                           /*!< Set WPS option */
	ESP_CMD_WIFI_MDNS,                          /*!< Configure MDNS function */
	ESP_CMD_WIFI_CWHOSTNAME_SET,                /*!< Set device hostname */
	ESP_CMD_WIFI_CWHOSTNAME_GET,                /*!< Get device hostname */
	/*
	 * TCP/IP related commands
	 */
	ESP_CMD_TCPIP_CIPSTATUS,                    /*!< Get status of connections */
	ESP_CMD_TCPIP_CIPDOMAIN,                    /*!< Get IP address from domain name = DNS function */
	ESP_CMD_TCPIP_CIPSTART,                     /*!< Start client connection */
	ESP_CMD_TCPIP_CIPSSLSIZE,                   /*!< Set SSL buffer size for SSL connection */
	ESP_CMD_TCPIP_CIPSEND,                      /*!< Send network data */
	ESP_CMD_TCPIP_CIPCLOSE,                     /*!< Close active connection */
	ESP_CMD_TCPIP_CIFSR,                        /*!< Get local IP */
	ESP_CMD_TCPIP_CIPMUX,                       /*!< Set single or multiple connections */
	ESP_CMD_TCPIP_CIPSERVER,                    /*!< Enables/Disables server mode */
	ESP_CMD_TCPIP_CIPSERVERMAXCONN,             /*!< Sets maximal number of connections allowed for server population */
	ESP_CMD_TCPIP_CIPMODE,                      /*!< Transmission mode, either transparent or normal one */
	ESP_CMD_TCPIP_CIPSTO,                       /*!< Sets connection timeout */
	ESP_CMD_TCPIP_PING,                         /*!< Ping domain */
	ESP_CMD_TCPIP_CIUPDATE,                     /*!< Perform self-update */
	ESP_CMD_TCPIP_CIPSNTPCFG,                   /*!< Configure SNTP servers */
	ESP_CMD_TCPIP_CIPSNTPTIME,                  /*!< Get current time using SNTP */
	ESP_CMD_TCPIP_CIPDNS,                       /*!< Configure user specific DNS servers */
	ESP_CMD_TCPIP_CIPDINFO,                     /*!< Configure what data are received on +IPD statement */
}ESP_ATComTypes_e;
typedef enum
{
	AT_SEND_OK = 0x00,
	AT_SEND_TIMEOUT = 0x01,
	AT_RECBUF_ERROR = 0x02,
}ESP_ATComSend_State_e;
typedef enum
{
	CONFIG_OK = 0x00,
	CONFIG_FAILED = 0x01,
	
	
	UNKNOWN_CMD = 0xFF,
}ESP_Config_State_t;
typedef enum
{
	REC_OK = 0x00,
	REC_TIMEOUT = 0x01,
	
}ESP_Receive_State_t;
typedef enum
{
	ESP_STATION = 0x00,
	ESP_AP = 0X01,
	ESP_MIX = 0x02,
	
	UNKNOWN_MODE = 0xFF,
}ESP_Mode_e;


typedef struct
{
	uint8_t SelectSSID;
	uint8_t SSIDCnt;
	int8_t  *SSIDSignalStrength;
	uint8_t **SSIDName;
	uint8_t **SSIDMac;
	uint8_t SSIDPassword[16];
}ESP_ScanInfor_t;

typedef struct
{
	bool TCPServerState;
	uint8_t ServerIP[4];
	uint16_t PortNum;
}ESP_TCPServerInfor_t;
typedef struct
{
	bool TCPClientState;
}ESP_TCPClientInfor_t;
typedef struct
{
	uint8_t *ESP_RecBuf;
	int16_t ESP_RealDataLength;
	int16_t ESP_RealDataAdd;
	uint8_t *ESP_RealData;
	ESP_ScanInfor_t ESP_ScanInfor;
	ESP_TCPServerInfor_t ESP_TCPServerInfor;
	ESP_TCPClientInfor_t ESP_TCPClientInfor;
}ESP_General_Control_t;


extern bool ESP_InitControlHandle(ESP_General_Control_t *Handle,uint16_t BuffSize);
extern ESP_Mode_e ESP_GetWorkMode(ESP_General_Control_t *Handle);
extern bool ESP_SetWorkMode(ESP_General_Control_t *Handle,ESP_Mode_e Mode);
extern ESP_Config_State_t ESP_Config(ESP_General_Control_t *Handle,ESP_ATComTypes_e Command);
extern bool ESP_ScanWIFI(ESP_General_Control_t *Handle);
extern bool ESP_ConnectWifi(ESP_General_Control_t *Handle,uint8_t *SSID,uint8_t *Password);
extern bool ESP_DisconnectWifi(ESP_General_Control_t *Handle);
extern bool ESP_GetIP(ESP_General_Control_t *Handle);
bool ESP_ConnectServerAsClient(ESP_General_Control_t *Handle,uint8_t *Server,uint16_t PortNum);
ESP_Receive_State_t ESP_ReceiveData(ESP_General_Control_t *Handle,uint8_t *Buffer,uint16_t ReadLength,uint32_t Timeout);
bool ESP_SendDataAsClient(ESP_General_Control_t *Handle,uint8_t *Data,uint16_t Length);

/**
 * \ingroup         ESP
 * \defgroup        ESP_UTILITIES Utilities
 * \brief           Utilities
 * \{
 */
 
/**
 * \brief           Assert an input parameter if in valid range
 * \note            Since this is a macro, it may only be used on a functions where return status is of type \ref espr_t enumeration
 * \param[in]       msg: message to print to debug if test fails
 * \param[in]       c: Condition to test
 */
#define ESP_ASSERT(msg, c)   do {   \
    if (!(c)) {                     \
        ESP_DEBUGF(ESP_CFG_DBG_ASSERT, "Wrong parameters on file %s and line %d: %s\r\n", __FILE__, __LINE__, msg); \
        return espPARERR;           \
    }                               \
} while (0)

/**
 * \brief           Align `x` value to specific number of bytes, provided by \ref ESP_CFG_MEM_ALIGNMENT configuration
 * \param[in]       x: Input value to align
 * \return          Input value aligned to specific number of bytes
 * \hideinitializer
 */
#define ESP_MEM_ALIGN(x)                    ((x + (ESP_CFG_MEM_ALIGNMENT - 1)) & ~(ESP_CFG_MEM_ALIGNMENT - 1))

/**
 * \brief           Get minimal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Minimal value between x and y parameters
 * \hideinitializer
 */
#define ESP_MIN(x, y)                       ((x) < (y) ? (x) : (y))

/**
 * \brief           Get maximal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Maximal value between `x` and `y` parameters
 * \hideinitializer
 */
#define ESP_MAX(x, y)                       ((x) > (y) ? (x) : (y))

/**
 * \brief           Get size of statically declared array
 * \param[in]       x: Input array
 * \return          Number of array elements
 * \hideinitializer
 */
#define ESP_ARRAYSIZE(x)                    (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           Unused argument in a function call
 * \note            Use this on all parameters in a function which are not used to prevent
 *                  compiler warnings complaining about "unused variables"
 * \param[in]       x: Variable which is not used
 * \hideinitializer
 */
#define ESP_UNUSED(x)                       ((void)(x))

/**
 * \brief           Get input value casted to `unsigned 32-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define ESP_U32(x)                          ((uint32_t)(x))

/**
 * \brief           Get input value casted to `unsigned 16-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define ESP_U16(x)                          ((uint16_t)(x))

/**
 * \brief           Get input value casted to `unsigned 8-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define ESP_U8(x)                           ((uint8_t)(x))

/**
 * \brief           Get input value casted to `size_t` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define ESP_SZ(x)                           ((size_t)(x))
#endif
