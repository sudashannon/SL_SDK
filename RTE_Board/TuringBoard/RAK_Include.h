#ifndef __WIFI_INCLUDE_H
#define __WIFI_INCLUDE_H
#define RW_OK                           (0)    //DONE
#define RW_ERR                          (-1)

/* define all error */
#define RW_ERR_PARAM_INVAILD            (-2)
#define RW_ERR_INIT_DRIVER_FAILD        (-3)
#define RW_ERR_DEINIT_DRIVER_FAILD      (-4)
#define RW_ERR_NO_DRIVER                (-5)
#define RW_ERR_NO_MEMORY                (-6)

#define RW_ERR_INVAILD_SSID             (-7)
#define RW_ERR_INVAILD_PSK              (-8)
#define RW_ERR_INVAILD_CHANNEL          (-9)
#define RW_ERR_INVAILD_SEC_MODE         (-10)    
#define RW_ERR_INVAILD_AUTH_MODE        (-11) 
#define RW_ERR_INVAILD_ROLE_MODE        (-12)
#define RW_ERR_INVAILD_CONFIG_MODE      (-13)  
    
#define RW_ERR_CMD_PENDING              (-14)
#define RW_ERR_NO_CONNECT               (-15)
#define RW_ERR_NOT_FIND_SSID            (-16)
#define RW_ERR_IP_DHCP                  (-17)

#define RW_ERR_SOCKET_INVAILD           (-18)
#define RW_ERR_SEND_BUFFER_FULL         (-19)
#define RW_ERR_TIME_OUT                 (-20)


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
typedef uint32_t rw_stamp_t;
#define   RW_MAX_SSID_LEN                      32
#define   RW_BSSID_LEN                         6
#define   RW_MAX_PASSPHRASE_SIZE               64
#define   RW_MIN_PASSPHRASE_SIZE               5
#define   RW_PMK_LEN                           32
#define   MAX_SEND_PACKET_LEN                  1400
#define   MAX_RECV_PACKET_LEN                  1024   // UDP: 1536  TCP:1024
#define   MAX_STAMP_DIFF                       0x7fffffff;

#define CONN_STATUS_STA_CONNECTED               0
#define CONN_STATUS_STA_DISCONNECT              1
#define CONN_STATUS_AP_ESTABLISH                2 //ap establish
#define CONN_STATUS_AP_CLT_CONNECTED            3 //client connect
#define CONN_STATUS_AP_CLT_DISCONNECT           4 //client disconnect
/*
 * Disconnect Event
 */
typedef enum {
    RW_NO_NETWORK_AVAIL   = 0x01,     /*  not find the match AP,same ssid or authmode */
    RW_LOST_LINK          = 0x02,     /* bmiss */
    RW_DISCONNECT_CMD     = 0x03,
    RW_BSS_DISCONNECTED   = 0x04,
    RW_AUTH_FAILED        = 0x05,
    RW_ASSOC_FAILED       = 0x06,
    RW_NO_RESOURCES_AVAIL = 0x07,
    RW_CSERV_DISCONNECT   = 0x08,
    RW_INVALID_PROFILE    = 0x0a,
    RW_DOT11H_CHANNEL_SWITCH = 0x0b,
    RW_PROFILE_MISMATCH   = 0x0c,
    RW_CONNECTION_EVICTED = 0x0d,
    RW_IBSS_MERGE         = 0xe
} RW_DISCONNECT_REASON;
typedef enum
{    
    ROLE_STA = 0,
    ROLE_AP,
    ROLE_ADHOC  // not support 
}rw_WlanMode_t;
typedef enum
{     
    RW_SEC_TYPE_OPEN = 0,
    RW_SEC_TYPE_SEC
}rw_SecMode_t;
typedef enum
{    
    RW_AUTH_TYPE_AUTO = 0, 
    RW_AUTH_TYPE_WPA_PSK_TKIP,
    RW_AUTH_TYPE_WPA_PSK_AES,
    RW_AUTH_TYPE_WPA2_PSK_TKIP,
    RW_AUTH_TYPE_WPA2_PSK_AES
}rw_AuthMode_t;
typedef enum
{    
    CONFIG_EASY = 0,
    CONFIG_WPS  
}rw_ConfigMode_t;
typedef enum
{    
    POWER_MAX = 0,
    POWER_SAVE  
}rw_PowerMode_t;
//==================================================================
// 网络信息结构体
//==================================================================
typedef struct {    
    uint8_t             channel; //网络信道
    uint8_t             ssid_len;//网络名称字符串长度
    uint8_t             rssi;//信号强度
    uint8_t             bssid[RW_BSSID_LEN];//网络bssid
    uint8_t             ssid[RW_MAX_SSID_LEN + 1];//网络ssid
    rw_SecMode_t        sec_mode;//是否加密
    rw_AuthMode_t       auth_mode; //加密方式
}rw_WlanNetworkInfo_t;
typedef struct {   
    int32_t num;         //有效网络个数
    rw_WlanNetworkInfo_t*  WlanNetworkInfo;     
}rw_WlanNetworkInfoList_t;
//==================================================================
// 网络连接/创建结构体
//==================================================================
typedef struct {
    rw_WlanMode_t       role_mode; //网络模式
    uint8_t             channel;   // used to point which channel to connect or creat  
    rw_SecMode_t        sec_mode;  //是否加密
    rw_AuthMode_t       auth_mode; //加密方式
    char*               psk;       //网络密码
    char*               pmk;     //32B hex,64B ascii ,used for auth_mode WPA,WPA2, NULL: don't used
    char*               ssid;    // can not be NULL
    uint8_t*            bssid;   //NULL: don't used
}rw_WlanConnect_t;
//==================================================================
// IP结构体
//==================================================================
typedef enum//ip配置模式
{    
    IP_CONFIG_STATIC = 0,//静态
    IP_CONFIG_QUERY,//
    DHCP_CLIENT,//dhcp客户端 用于ap
    DHCP_SERVER //dhcp服务器 用于sta
}rw_IpConfigMode_t;
typedef struct {
    uint32_t            addr;
    uint32_t            mask;
    uint32_t            gw;
    uint32_t            svr1;//dns 1
    uint32_t            svr2;//dns 2 
}rw_IpConfig_t;
//==================================================================
// wps easy配置结构体
//==================================================================
typedef struct {
    uint8_t             bssid[RW_BSSID_LEN];
    uint8_t             ssid[RW_MAX_SSID_LEN]; 
    uint8_t             psk[RW_MAX_PASSPHRASE_SIZE];
    uint8_t             channel;
}rw_WlanEasyConfigWpsResponse_t;
//==================================================================
// LIB配置结构体
//==================================================================
typedef uint32_t(*rw_HwInit_)(void);               
typedef void(*rw_PowerUpDown_)(uint8_t status);
typedef void*(*rw_Malloc_)(uint32_t size);
typedef void(*rw_Free_)(void* data);
typedef void(*rw_TimeDelay_)(int ms);
typedef rw_stamp_t(*rw_Stamp_)(void);
typedef void(*rw_ToggleIrq_)(uint8_t enable);
typedef void(*rw_SpiIoBuffer_)(uint8_t* write, uint8_t* read, uint16_t len); 
typedef void(*rw_AssertFunc_)(const char* file, int line);
struct driver_cb_
{
    rw_HwInit_          hw_init;           
    rw_HwInit_          hw_deinit;
    rw_PowerUpDown_     hw_power;
    rw_Malloc_          driver_malloc;
    rw_Free_            driver_free;
    rw_TimeDelay_       time_delay;
    rw_Stamp_           Stamp_get;
    rw_ToggleIrq_       toggle_irq;
    rw_SpiIoBuffer_     spi_io_buffer;
    rw_AssertFunc_      driver_assert;
};
typedef void(*rw_WlanConnEvent_)(uint8_t event, 
                                 rw_WlanConnect_t* wlan_info, 
                                 RW_DISCONNECT_REASON dis_reasoncode);

typedef void(*rw_WlanScan_)(rw_WlanNetworkInfoList_t* scan_info);
typedef void(*rw_WlanEasyWps_)(rw_WlanEasyConfigWpsResponse_t*  pResponse, int status);
typedef void(*rw_IpDhcp_)(rw_IpConfig_t* addr, int status);
typedef void(*rw_DnsResult_) (int dnsIp);
//typedef void(*rw_TcpcResult_) (int sockfd , int result);
struct app_cb_
{
    rw_WlanConnEvent_   conn_cb; 
    rw_WlanScan_        scan_cb;
    rw_WlanEasyWps_     easy_wps_cb;
    rw_IpDhcp_          dhcp_cb;
    rw_DnsResult_       dns_cb;
    //rw_TcpcResult_      tcpc_cb;
};
typedef struct
{
    bool                spi_int_enable;          // customer can choose enable or disenable spi int event driver    
    uint8_t             rx_queue_num;            // rx buffer queue num >=1
    uint8_t             socket_max_num;          // module support socket numbers max 8
    uint8_t             scan_max_num;            // scan result buffer numbers normal:10 if you need more can raise it 
    uint8_t             tcp_retry_num;           // tcp backoff retry numbers
    char*               host_name;               // module host name ,you can see it in router clients when dhcp
    char*               country_code;            // set module country code ,CN (1-13),JS(1-14）,UP（1-11）
    struct              driver_cb_ driver_cb;    // platform related driver used
    struct              app_cb_ app_cb;          // application related callback info
    uint8_t             tx_queue_num;            // tx buffer queue MUST be >=1
    uint16_t            tx_max_len;              // tx max len MUST be <= 1400
}rw_DriverParams_t;

/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to load customer's driver parameters.
*                   and power up ,reset ,hal,initialize driver to make the wifi ready . 
*
* Arguments       : rw_DriverParams_t 
*
* Returned Value  : RW_ERR_INIT_DRIVER_FAILD    -- initialize driver failed
*                   RW_ERR_PARAM_INVAILD        -- parameter  invaild please check
*                   RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//驱动初始化
int rw_sysDriverInit(rw_DriverParams_t* params);


/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to found around APs ,
*                    you can pointed ssid or channel if you need.
*                   The scan result will return to the callback function .
*
* Arguments       : pssid                       -- pointed ssid   NULL: all
*                   channel                     -- pointed channel 1-14 ,0: all
*
* Returned Value  : RW_ERR_INVAILD_CHANNEL      -- channel parameter invaild
*                   RW_ERR_INVAILD_SSID         -- ssid parameter invaild
*                   RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
* 
*END*-----------------------------------------------------------------*/
//扫描网络 获取扫描信息
int rw_wlanNetworkScan(char* pssid, int channel);
int rw_wlanGetScanInfo(rw_WlanNetworkInfoList_t *pInfoList);

/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to config ap if you need hidden it  
*
* Arguments       : is_hidden   1- hidden  0- not hidden broadcast enable 
*
* Returned Value  : RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//ap 配置
int rw_wlanApConfig(uint8_t is_hidden);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to connect to the pointed ssid AP or 
*                  creat the AP . the parameters of AP , you can pointed set or 
*                  set auto ,driver will find the signal stronger and connect .
*                  this command is unblock ,only set the parameters .
*
* Arguments       : rw_WlanConnect_t    
*
* Returned Value  : 
*                   RW_ERR_INVAILD_ROLE_MODE    -- role mode parameter invaild 
*                   RW_ERR_INVAILD_SSID         -- ssid parameter invaild   
*                   RW_ERR_INVAILD_CHANNEL      -- channel parameter invaild
*                   RW_ERR_INVAILD_SEC_MODE     -- sec_mode parameter invaild
*                   RW_ERR_INVAILD_AUTH_MODE    -- auth_mode parameter invaild
*                   RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//连接 创建网络
int rw_wlanConnect(const rw_WlanConnect_t *conn);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to config ip parameters static or query
*                   module ,dhcp client to obtain ip info. Enable AP dhcp server
*
* Arguments       : rw_IpConfig_t ,  rw_IpConfigMode_t 
*
* Returned Value  : RW_ERR_IP_DHCP              -- ipdhcp fail or timeout
*                   RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//ip配置
int rw_ipConfig(rw_IpConfig_t* ip_addr, rw_IpConfigMode_t mode);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to resolv ip from domain in STA mode. 
*                   this command is unblock ,the result return to customer callback
*
* Arguments       : host_name ,  name_len , family 
*
* Returned Value  : RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//dns域名解析
int rw_dnsRequest(const char *host_name, uint8_t name_len, uint16_t family);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to config wifi to connect to pointed AP. 
*                   Easyconfig cooperate with app of android or ios .
*                   WPS need AP support this feature  WPS2.0
*
* Arguments       : rw_ConfigMode_t 
*
* Returned Value  : RW_ERR_INVAILD_CONFIG_MODE  -- config mode parameter invaild 
*                   RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//wps easy配置网络
int rw_startEasyWps(rw_ConfigMode_t mode);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to set the module power mode. 
*
* Arguments       : pwr_mode 
*                  1) POWER_MAX - the radio is always on either transmitting,
*                                receiving or listening
*                  2) POWER_SAVE - the wifi device will according to its built in rules 
*                                enter/exit sleep mode with the Access point.
*
* Returned Value  : RW_ERR_INVAILD_CONFIG_MODE  -- config mode parameter invaild 
*                   RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//设置功耗模式
int rw_setPwrMode(rw_PowerMode_t pwr_mode);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to get the PMK from the success connection 
*                   with the AP use auth mode WPA or WPA2 ,when Next connect you can 
*                   use the PMK instead of PSK , there will be fast to connect.
*
* Arguments       : pmk  32B hex or 64B ascii.
*
* Returned Value  : RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//获取pmk
int rw_getPMK(char* pmk);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to get the signal strong indicate with  
*                   the associated AP ,return the absolute value of negative  rssi                  
*
* Arguments       : none
*
* Returned Value  : RW_ERR                      -- cmd execute failed
*                   >0                          -- rssi value
*                     
*END*-----------------------------------------------------------------*/
//获取信号强度
int rw_getRSSI(void);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to get the mac of the module. 
*                   mac result in user define mac_data buffer
*
* Arguments       : mac_data buffer
*
* Returned Value  : RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//获取mac地址
int rw_getMacAddr(char* mac_data);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to get the version of the module. 
*                   version result in user define version buffer
*
* Arguments       : version buffer
*
* Returned Value  : RW_ERR                      -- cmd execute failed
*                   RW_OK                       -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//软件版本
int rw_getLibVersion(char* version);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to handle module interrupt . 
*                   customer should call it in interrupt-Handler
*
* Arguments       : none
*
* Returned Value  : none
*                     
*END*-----------------------------------------------------------------*/
//中断处理函数
void DRIVER_INT_HANDLE(void);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to disable and free wifi part function.
*                   power down ,deinit hal,uninstall the driver  . 
*
* Arguments       : none 
*
* Returned Value  : RW_ERR_DEINIT_DRIVER_FAILD    -- deinit driver failed
*                   RW_OK                         -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//驱动反初始化
int rw_sysDriverDeinit(void);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to rest wifi part function to init status.
*                   like the hard reset . 
*
* Arguments       : none 
*
* Returned Value  : RW_ERR_DEINIT_DRIVER_FAILD    -- deinit driver failed
*                   RW_ERR_INIT_DRIVER_FAILD      -- initialize driver failed
*                   RW_ERR_PARAM_INVAILD          -- parameter  invaild please check
*                   RW_ERR                        -- cmd execute failed
*                   RW_OK                         -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
//驱动复位
int rw_sysDriverReset(void);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to actually perform user's cmd and callback.
*                   info ,manage wifi events .this superloop should call frequence in 
*                   the unblock noneOS driver .
* Arguments       : none 
*
* Returned Value  : RW_ERR                        -- cmd execute failed
*                   RW_OK                         -- cmd execute success
*                     
*END*-----------------------------------------------------------------*/
#if !DRIVER_CONFIG_MULTI_TASKING  
//轮询
int rw_sysDriverLoop(void);
#endif
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to do system delay ms ,will can driver
*                   loop in it when unblock noneOS driver;
*
* Arguments       : ms 
*
* Returned Value  : none
*                     
*END*-----------------------------------------------------------------*/
void rw_sysSleep(int ms);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to set a futureStamp base the platfrm 
*                   tick, that customer can set a cmd timeout in unblock noneOS 
*                   mode .
* 
* Arguments       : stamp ,msec
*
* Returned Value  : none
*                     
*END*-----------------------------------------------------------------*/
void  rwSetFutureStamp(rw_stamp_t* stamp, uint32_t msec);
/*FUNCTION*-------------------------------------------------------------
*
* Description     : This function is used to judge that is there timeout of your
*                   set  futureStamp , if out return true ,other return false.
* 
* Arguments       : stamp 
*
* Returned Value  : true or false
*                     
*END*-----------------------------------------------------------------*/
bool  rwIsStampPassed(rw_stamp_t* stamp);
#endif
