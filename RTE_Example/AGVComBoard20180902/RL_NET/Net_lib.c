/*----------------------------------------------------------------------------
 *      RL-ARM - TCPnet
 *----------------------------------------------------------------------------
 *      Name:    NET_LIB.C
 *      Purpose: TCPnet System Configuration
 *      Rev.:    V4.72
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#pragma thumb
#pragma O3

/*----------------------------------------------------------------------------
 *      Library for Net_Config.c
 *---------------------------------------------------------------------------*/

#ifdef __NET_CONFIG__

/* Non-Critical Net_Config.c upgrade */
#ifndef HTTP_SRVID
 #define HTTP_SRVID     ""
#endif
#ifndef FTP_WELMSG
 #define FTP_WELMSG     ""
#endif
#ifndef DHCP_VCID
 #define DHCP_VCID      ""
#endif
#ifndef DHCP_BOOTF
 #define DHCP_BOOTF     0
#endif
#ifndef DHCP_NTPSRV
 #define DHCP_NTPSRV    0
#endif
#ifndef TNET_NOECHO
 #define TNET_NOECHO    0
#endif
#ifndef TFTP_ENFWALL
 #define TFTP_ENFWALL   0
#endif
#ifndef TCP_MAXSEGSZ
 #define TCP_MAXSEGSZ   1460
#endif
#ifndef TCP_RECWINSZ
 #define TCP_RECWINSZ   TCP_DEF_WINSIZE
#endif

#define TICK_RATE       (1000 / TICK_INTERVAL)

/* Some configuration error checking. */
#if !(SLIP_ENABLE || PPP_ENABLE || ETH_ENABLE)
 #error All network interfaces disabled in configuration
#endif

#if (SLIP_ENABLE && PPP_ENABLE)
 #error Both SLIP and PPP interfaces enabled
#endif

/* Check TCP Socket enable. */
#if (!TCP_ENABLE & (BSD_ENABLE | HTTP_ENABLE | TNET_ENABLE | FTP_ENABLE | \
                    FTPC_ENABLE | SMTP_ENABLE))
 #error Application needs TCP Sockets enabled
#endif

/* Check UDP Socket enable */
#if (!UDP_ENABLE & (BSD_ENABLE | TFTP_ENABLE | DNS_ENABLE | SNMP_ENABLE | \
                    SNTP_ENABLE | ((DHCP_ENABLE | NBNS_ENABLE) & ETH_ENABLE)))
 #error Application needs UDP Sockets enabled
#endif

/* Check number of TCP sockets available. */
#define __TCPNS    ((BSD_ENABLE  * BSD_NUMSOCKS)   + \
                    (HTTP_ENABLE * HTTP_NUMSESS)   + \
                    (TNET_ENABLE * TNET_NUMSESS)   + \
                    (FTP_ENABLE  * FTP_NUMSESS*2)  + \
                    (FTPC_ENABLE * 2)              + \
                    (SMTP_ENABLE * 1))

#if (__TCPNS > TCP_NUMSOCKS)
 #error Number of TCP Sockets too small
#endif

/* Calculate number of UDP sockets needed for TFTP server. */
#if (TFTP_ENFWALL)
 #define __TFTP_NSOCKS  1
#else
 #define __TFTP_NSOCKS  (1 + TFTP_NUMSESS)
#endif

/* Check number of UDP sockets available. */
#define __UDPNS    ((BSD_ENABLE  * BSD_NUMSOCKS)   + \
                    (TFTP_ENABLE * __TFTP_NSOCKS)  + \
                    (TFTPC_ENABLE* 1)              + \
                    (DNS_ENABLE  * 1)              + \
                    (SNMP_ENABLE * 1)              + \
                    (SNTP_ENABLE * 1)              + \
                    (DHCP_ENABLE * ETH_ENABLE)     + \
                    (NBNS_ENABLE * ETH_ENABLE))

#if (__UDPNS > UDP_NUMSOCKS)
 #error Number of UDP Sockets too small
#endif

/* Check BSD Server sockets */
#if (BSD_ENABLE && (BSD_SRVSOCKS > BSD_NUMSOCKS))
 #error Number of BSD Server sockets too large
#endif

/* Check BSD Hostname resolver */
#if (BSD_ENABLE && BSD_GETHOSTEN && !DNS_ENABLE)
 #error BSD Hostname resolver needs DNS enabled
#endif

/* Check PPP Authentication protocols */
#if (PPP_ENABLE && PPP_AUTHEN && !(PPP_PAPEN | PPP_CHAPEN))
 #error PPP Authentication needs PAP or/and CHAP enabled
#endif

/* Check Net_Config.c version */
#if (TNET_ENABLE && !defined(TNET_IDLETOUT))                     || \
    (FTP_ENABLE  && !defined(FTP_IDLETOUT))                      || \
    (PPP_ENABLE  && (defined(_ACCM1) || defined(LCP_MAXRETRY)))
 #error Net_Config.c too old, needs update
#endif

/* Check Maximum Segment Size of TCP Socket */
#if (TCP_ENABLE && ((TCP_MAXSEGSZ < 536) || (TCP_MAXSEGSZ > 1460)))
 #error TCP Maximum Segment Size out of range
#endif

/* Check Receive Window Size of TCP Socket */
#if (TCP_ENABLE && (TCP_RECWINSZ < TCP_MAXSEGSZ))
 #error TCP Receive Window Size too small
#endif

/* Check Block Size of TFTP Server */
#if (TFTPC_ENABLE && ((TFTPC_BLOCKSZ < 128) || (TFTPC_BLOCKSZ > 1428)))
 #error TFTP Client Block Size out of range
#endif

/* Local Machine info */
LOCALM const nlocalm[3] = {
 /* Ethernet: Local IP address */
#if (ETH_ENABLE)
 {{_IP1,_IP2,_IP3,_IP4},
  /* Default Gateway IP address */
  {_GW1,_GW2,_GW3,_GW4},
  /* Subnet IP Mask */
  {_MSK1,_MSK2,_MSK3,_MSK4},
  /* Primary DNS Server IP */
  {_pDNS1,_pDNS2,_pDNS3,_pDNS4},
  /* Secondary DNS Server IP */
  {_sDNS1,_sDNS2,_sDNS3,_sDNS4}},
#else
 {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
#endif
 /* PPP: Local IP address */
#if (PPP_ENABLE)
 {{_IP1P,_IP2P,_IP3P,_IP4P},
  /* Default Gateway IP address */
  {0,0,0,0},
  /* Subnet IP Mask */
  {_MSK1P,_MSK2P,_MSK3P,_MSK4P},
  /* Primary DNS Server IP */
  {_pDNS1P,_pDNS2P,_pDNS3P,_pDNS4P},
  /* Secondary DNS Server IP */
  {_sDNS1P,_sDNS2P,_sDNS3P,_sDNS4P}},
#else
 {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
#endif
 /* SLIP: Local IP address */
#if (SLIP_ENABLE)
 {{_IP1S,_IP2S,_IP3S,_IP4S},
  /* Default Gateway IP address */
  {0,0,0,0},
  /* Subnet IP Mask */
  {_MSK1S,_MSK2S,_MSK3S,_MSK4S},
  /* Primary DNS Server IP */
  {_pDNS1S,_pDNS2S,_pDNS3S,_pDNS4S},
  /* Secondary DNS Server IP */
  {_sDNS1S,_sDNS2S,_sDNS3S,_sDNS4S}}
#else
 {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
#endif
};
/* System configuration */
static uint32_t mem_pool[MEM_SIZE];
       uint8_t  lhost_name[16] = LHOST_NAME;
SYS_CFG sys_config = {
  mem_pool,
  sizeof (mem_pool),
  TICK_RATE,
  TICK_INTERVAL,
  TICK_RATE / 5,
  (PPP_GETIP<<5   | SLIP_DEFGW<<4 | PPP_DEFGW<<3 |
   SLIP_ENABLE<<2 | PPP_ENABLE<<1 | ETH_ENABLE),
  lhost_name
};
/* Ethernet network interface */
#if (ETH_ENABLE)
 static ARP_INFO arp_table[ARP_TABSIZE];
 uint8_t     own_hw_adr[ETH_ADRLEN] = {_MAC1,_MAC2,_MAC3,_MAC4,_MAC5,_MAC6};
 ARP_CFG arp_config = {
   arp_table,
   ARP_TABSIZE,
   ARP_TIMEOUT,
   ARP_MAXRETRY,
   ARP_RESEND,
   ARP_NOTIFY
 };
#endif
#if (PPP_ENABLE)
 PPP_CFG ppp_config = {
   PPP_ACCM,
   5 * TICK_RATE,
   PPP_RETRYTOUT * TICK_RATE,
   PPP_ECHOTOUT,
   PPP_MAXRETRY,
   ((PPP_CHAPEN << 1) | PPP_PAPEN) * PPP_AUTHEN
 };
#endif
#if (SLIP_ENABLE)
 uint16_t const slip_SerTout  = 5 * TICK_RATE;
#endif
#if (ETH_ENABLE && IGMP_ENABLE)
 static IGMP_INFO igmp_table[IGMP_TABSIZE];
 IGMP_CFG  igmp_config = {
   igmp_table,
   IGMP_TABSIZE,
 };
#endif
#if (ETH_ENABLE && DHCP_ENABLE)
 DHCP_CFG dhcp_config = {
   DHCP_VCID,
   (DHCP_NTPSRV << 1) | DHCP_BOOTF
 };
#endif
#if (UDP_ENABLE)
 static UDP_INFO udp_scb[UDP_NUMSOCKS];
 UDP_CFG udp_config = {
   udp_scb,
   UDP_NUMSOCKS,
 };
#endif
#if (TCP_ENABLE)
 static TCP_INFO tcp_scb[TCP_NUMSOCKS];
 TCP_CFG tcp_config = {
   tcp_scb,
   TCP_NUMSOCKS,
   TCP_MAXRETRY,
   TCP_RETRYTOUT * TICK_RATE,
   (200 + TICK_INTERVAL - 1) / TICK_INTERVAL,
   TCP_SYN_RETRY_TOUT * TICK_RATE,
   TCP_INIT_RETRY_TOUT * TICK_RATE,
   TCP_DEFTOUT,
   TCP_MAXSEGSZ,
   TCP_RECWINSZ,
   TCP_CONRETRY
 };
#endif
#if (HTTP_ENABLE)
 static HTTP_INFO http_scb[HTTP_NUMSESS];
 static uint8_t http_passw[PASSW_SZ] = HTTP_AUTHPASSW;
 HTTP_CFG  http_config = {
   http_scb,
   HTTP_NUMSESS,
   HTTP_ENAUTH,
   HTTP_PORTNUM,
   HTTP_SRVID,
 #if (HTTP_ENAUTH)
   HTTP_AUTHREALM,
   HTTP_AUTHUSER,
   http_passw
 #else
   "","",""
 #endif
 };
#endif
#if (TNET_ENABLE)
 static TNET_INFO tnet_scb[TNET_NUMSESS];
 static uint8_t tnet_passw[PASSW_SZ] = TNET_AUTHPASSW;
 TNET_CFG  tnet_config = {
   tnet_scb,
   TNET_NUMSESS,
   TNET_ENAUTH,
   TNET_NOECHO,
   TNET_PORTNUM,
   TNET_IDLETOUT,
 #if (TNET_ENAUTH)
   TNET_AUTHUSER,
   tnet_passw
 #else
   "",""
 #endif
 };
#endif
#if (TFTP_ENABLE)
 static TFTP_INFO tftp_scb[TFTP_NUMSESS];
 TFTP_CFG tftp_config = {
   &tftp_scb[0],
   TFTP_NUMSESS,
   TFTP_MAXRETRY,
   TFTP_PORTNUM,
   TFTP_DEFTOUT,
   TFTP_ENFWALL
 };
#endif
#if (TFTPC_ENABLE)
 TFTPC_CFG tftpc_config = {
   TFTPC_BLOCKSZ,
   (TFTPC_RETRYTO * TICK_RATE + 5) / 10,
   TFTPC_MAXRETRY
 };
#endif
#if (FTP_ENABLE)
 static FTP_INFO ftp_scb[FTP_NUMSESS];
 static uint8_t ftp_passw[PASSW_SZ] = FTP_AUTHPASSW;
 #define _LS(s)  sizeof(s)-1,s
 FTP_CFG ftp_config = {
   ftp_scb,
   FTP_NUMSESS,
   FTP_ENAUTH,
   FTP_PORTNUM,
   FTP_IDLETOUT,
  _LS(FTP_WELMSG),
 #if (FTP_ENAUTH)
   FTP_AUTHUSER,
   ftp_passw
 #else
   "",""
 #endif
 };
#endif
#if (FTPC_ENABLE)
 FTPC_CFG ftpc_config = {
   FTPC_DEFTOUT,
   FTPC_PASVMODE
 };
#endif
#if (DNS_ENABLE)
 static DNS_CACHE dns_table[DNS_TABSIZE];
DNS_CFG dns_config = {
   dns_table,
   DNS_TABSIZE
 };
#endif
#if (SMTP_ENABLE)
 uint8_t const smtp_DefTout = SMTP_DEFTOUT;
#endif
#if (SNMP_ENABLE)
 SNMP_CFG snmp_config = {
   SNMP_PORTNUM,
   SNMP_TRAPPORT,
   { SNMP_TRAPIP1, SNMP_TRAPIP2, SNMP_TRAPIP3, SNMP_TRAPIP4 },
   SNMP_COMMUNITY,
   TICK_INTERVAL / 5
 };
#endif
#if (SNTP_ENABLE)
 SNTP_CFG sntp_config = {
 { SNTP_SRVIP1, SNTP_SRVIP2, SNTP_SRVIP3, SNTP_SRVIP4 },
 SNTP_BCMODE
 };
#endif
#if (BSD_ENABLE) 
 static BSD_INFO bsd_scb[BSD_NUMSOCKS + BSD_SRVSOCKS];
 #ifdef __RTX
  static osMutexId_t bsd_mutex;
  static osEventFlagsId_t bsd_evt; 
  #define BSD_INRTX  true
 #else
  #define BSD_INRTX  false
 #endif
 BSD_CFG bsd_config = {
   bsd_scb,
   BSD_NUMSOCKS + BSD_SRVSOCKS,
   BSD_INRTX,
   BSD_RCVTOUT * TICK_RATE
 };
#endif
/*----------------------------------------------------------------------------
 *      Library Interface
 *---------------------------------------------------------------------------*/


/*--------------------------- init_system -----------------------------------*/

void init_system (void) {
  /* Initialize configured interfaces and applications. */

#if (ETH_ENABLE)
  eth_init_link ();
#endif
#if (PPP_ENABLE)
  ppp_init_link ();
#endif
#if (SLIP_ENABLE)
  slip_init_link ();
#endif
  ip_init ();
  icmp_init ();
#if (ETH_ENABLE && IGMP_ENABLE)
  igmp_init ();
#endif
#if (UDP_ENABLE)
  udp_init ();
#endif
#if (TCP_ENABLE)
  tcp_init ();
#endif
#if (BSD_ENABLE)
  bsd_init ();
 #if (BSD_GETHOSTEN)
  bsd_init_host ();
 #endif
#endif
#if (HTTP_ENABLE)
  http_init ();
#endif
#if (TNET_ENABLE)
  tnet_init ();
#endif
#if (TFTP_ENABLE)
  tftp_init ();
#endif
#if (TFTPC_ENABLE)
  tftpc_init ();
#endif
#if (FTP_ENABLE)
  ftp_init ();
#endif
#if (FTPC_ENABLE)
  ftpc_init ();
#endif
#if (ETH_ENABLE && NBNS_ENABLE)
  nbns_init ();
#endif
#if (ETH_ENABLE && DHCP_ENABLE)
  dhcp_init ();
#elif (ETH_ENABLE)
  arp_notify ();
#endif
#if (DNS_ENABLE)
  dns_init ();
#endif
#if (SMTP_ENABLE)
  smtp_init ();
#endif
#if (SNMP_ENABLE)
  snmp_init ();
#endif
#if (SNTP_ENABLE)
  sntp_init ();
#endif

#if (BSD_ENABLE && __RTX)
   bsd_mutex = osMutexNew(NULL);
	 bsd_evt = osEventFlagsNew(NULL);
#endif
}


/*--------------------------- run_system ------------------------------------*/

void run_system (void) {
  /* Run configured interfaces and applications. */

#if (BSD_ENABLE && __RTX)
  osMutexAcquire (bsd_mutex, osWaitForever);
#endif

#if (ETH_ENABLE)
  eth_run_link ();
#endif
#if (PPP_ENABLE)
  ppp_run_link ();
#endif
#if (SLIP_ENABLE)
  slip_run_link ();
#endif
  ip_run_local ();
  icmp_run_engine ();
#if (ETH_ENABLE && IGMP_ENABLE)
  igmp_run_host ();
#endif
#if (TCP_ENABLE)
  tcp_poll_sockets ();
#endif
#if (BSD_ENABLE)
  bsd_poll_sockets ();
#endif
#if (HTTP_ENABLE)
  http_run_server ();
#endif
#if (TNET_ENABLE)
  tnet_run_server ();
#endif
#if (TFTP_ENABLE)
  tftp_run_server ();
#endif
#if (TFTPC_ENABLE)
  tftpc_run_client ();
#endif
#if (FTP_ENABLE)
  ftp_run_server ();
#endif
#if (FTPC_ENABLE)
  ftpc_run_client ();
#endif
#if (ETH_ENABLE && DHCP_ENABLE)
  dhcp_run_client ();
#endif
#if (DNS_ENABLE)
  dns_run_client ();
#endif
#if (SMTP_ENABLE)
  smtp_run_client ();
#endif
#if (SNMP_ENABLE)
  snmp_run_agent ();
#endif
#if (SNTP_ENABLE)
  sntp_run_client ();
#endif

#if (BSD_ENABLE && __RTX)
  osMutexRelease (bsd_mutex);
#endif
}


/*--------------------------- bsd_suspend/resume ----------------------------*/
static uint8_t ThreadID[4] = {0};
#if (BSD_ENABLE && __RTX)
__used void bsd_suspend (uint8_t *tsk_id) {
  /* Suspend a socket owner task. */
	*tsk_id = (uint8_t)osThreadGetId ();
  osMutexRelease (bsd_mutex);
	osEventFlagsWait (bsd_evt,*tsk_id,osFlagsWaitAny, osWaitForever);
  osMutexAcquire (bsd_mutex, osWaitForever);
}

__used void bsd_resume (uint8_t tsk_id) {
  /* Resume a task waiting for a socket event. */
  if (tsk_id) {
		osEventFlagsSet(bsd_evt, tsk_id);
  }
}
#endif


/*--------------------------- bsd_lock/unlock -------------------------------*/

#if (BSD_ENABLE && __RTX)
__used void bsd_lock (void) {
  /* Acquire mutex - Lock TCPnet functions. */
  osMutexAcquire (bsd_mutex, osWaitForever);
}

__used void bsd_unlock (void) {
  /* Release mutex - Unlock TCPnet functions. */
  osMutexRelease (bsd_mutex);
}
#endif


/*--------------------------- size optimization -----------------------------*/

#if !(ETH_ENABLE)
/* Empty functions when Ethernet Interface is disabled. */
bool eth_chk_adr (OS_FRAME *frame)   { return (false); }
uint8_t  *eth_get_adr (uint8_t *ipadr)         { return (NULL); }
bool eth_send_frame (OS_FRAME *frame){ return (false); }
#endif

#if !(ETH_ENABLE && IGMP_ENABLE)
/* Empty functions when IP Multicasting is not enabled. */
bool igmp_is_member (uint8_t *ipadr)      { return (false); }
void igmp_process (OS_FRAME *frame)  { ; }
#endif

#if (ETH_ENABLE && !DHCP_ENABLE)
/* Empty function when DHCP not enabled. */
void dhcp_disable (void)             { ; }
#endif

#if !(PPP_ENABLE)
/* Empty function when PPP Interface is disabled. */
bool ppp_send_frame (OS_FRAME *frame, uint16_t prot) { return (false); }
#endif

#if (PPP_ENABLE && !(PPP_AUTHEN && PPP_PAPEN))
/* Empty functions when PAP authentication not enabled. */
void pap_init (void)                 { ; }
void pap_run (void)                  { ; }
void pap_process (OS_FRAME *frame)   { ; }
#endif

#if (PPP_ENABLE && !(PPP_AUTHEN && PPP_CHAPEN))
/* Empty functions when CHAP authentication not enabled. */
void chap_init (void)                { ; }
void chap_run (void)                 { ; }
void chap_process (OS_FRAME *frame)  { ; }
#endif

#if !(SLIP_ENABLE)
/* Empty function when SLIP Interface is disabled. */
bool slip_send_frame (OS_FRAME *frame) { return (false); }
#endif

#if !(UDP_ENABLE)
/* Empty function when UDP socket not enabled. */
void udp_process (OS_FRAME *frame)   { ; }
#endif

#if !(TCP_ENABLE)
/* Empty function when TCP socket not enabled. */
void tcp_process (OS_FRAME *frame)   { ; }
#endif

#if !(SNTP_ENABLE)
/* Empty function when SNTP not enabled. */
bool sntp_get_time (uint8_t *ipadr, void (*cbfunc)(uint32_t)) { return(false); }
#endif

#if (BSD_ENABLE && !__RTX)
/* Empty functions for non RTX environment. */
__used uint8_t   bsd_wait   (BSD_INFO *bsd_s, uint8_t evt) { return (0);}
__used void bsd_enable (BSD_INFO *bsd_s, uint8_t evt) { ; }
#endif

#endif    /* __NET_CONFIG__ */


/*----------------------------------------------------------------------------
 *      Library for Net_Debug.c
 *---------------------------------------------------------------------------*/

#ifdef __NET_DEBUG__

/* Non-Critical Net_Debug.c upgrade */
#ifndef DBG_TIME
 #define DBG_TIME   1
#endif
#ifdef DBG_APP
 #define DBG_HTTP   DBG_APP
 #define DBG_FTP    DBG_APP
 #define DBG_FTPC   DBG_APP
 #define DBG_TNET   DBG_APP
 #define DBG_TFTP   DBG_APP
 #define DBG_TFTPC  DBG_APP
 #define DBG_SMTP   DBG_APP
#endif

#include <stdarg.h>
#include <stdio.h>

#pragma diag_suppress 111

extern char *dbg_time (void);
extern char *dbg_proc (uint32_t proc);

/*--------------------------- __debug__ -------------------------------------*/

void __debug__ (uint32_t proc, const char *fmt, ...) {
  /* Print TCPnet Debug messages. */
  va_list args;

  switch (proc) {
#if (DBG_MEM > 1)
    case MODULE_MEM:  break;
#endif
#if (DBG_ETH > 1)
    case MODULE_ETH:  break;
#endif
#if (DBG_PPP > 1)
    case MODULE_PPP:  break;
#endif
#if (DBG_SLIP > 1)
    case MODULE_SLIP: break;
#endif
#if (DBG_ARP > 1)
    case MODULE_ARP:  break;
#endif
#if (DBG_IP > 1)
    case MODULE_IP:   break;
#endif
#if (DBG_ICMP > 1)
    case MODULE_ICMP: break;
#endif
#if (DBG_IGMP > 1)
    case MODULE_IGMP: break;
#endif
#if (DBG_UDP > 1)
    case MODULE_UDP:  break;
#endif
#if (DBG_TCP > 1)
    case MODULE_TCP:  break;
#endif
#if (DBG_NBNS > 1)
    case MODULE_NBNS: break;
#endif
#if (DBG_DHCP > 1)
    case MODULE_DHCP: break;
#endif
#if (DBG_DNS > 1)
    case MODULE_DNS:  break;
#endif
#if (DBG_SNMP > 1)
    case MODULE_SNMP: break;
#endif
#if (DBG_BSD > 1)
    case MODULE_BSD:  break;
#endif
#if (DBG_HTTP > 1)
    case MODULE_HTTP: break;
#endif
#if (DBG_FTP > 1)
    case MODULE_FTP:  break;
#endif
#if (DBG_FTPC > 1)
    case MODULE_FTPC: break;
#endif
#if (DBG_TNET > 1)
    case MODULE_TNET: break;
#endif
#if (DBG_TFTP > 1)
    case MODULE_TFTP: break;
#endif
#if (DBG_TFTPC > 1)
    case MODULE_TFTPC:break;
#endif
#if (DBG_SMTP > 1)
    case MODULE_SMTP: break;
#endif
#if (DBG_SNTP > 1)
    case MODULE_SNTP: break;
#endif
    default: return;
  }
#if (DBG_TIME)
  printf (dbg_time());
#endif
  printf ("%s:",dbg_proc(proc));

  va_start (args,fmt);
  vprintf (fmt,args);
  va_end (args);
}


/*--------------------------- __error__ -------------------------------------*/

void __error__ (uint32_t proc, const char *fmt, ...) {
  /* Print TCPnet Error messages. */
  va_list args;

  switch (proc) {
#if (DBG_MEM > 0)
    case MODULE_MEM:  break;
#endif
#if (DBG_ETH > 0)
    case MODULE_ETH:  break;
#endif
#if (DBG_PPP > 0)
    case MODULE_PPP:  break;
#endif
#if (DBG_SLIP > 0)
    case MODULE_SLIP: break;
#endif
#if (DBG_ARP > 0)
    case MODULE_ARP:  break;
#endif
#if (DBG_IP > 0)
    case MODULE_IP:   break;
#endif
#if (DBG_ICMP > 0)
    case MODULE_ICMP: break;
#endif
#if (DBG_IGMP > 0)
    case MODULE_IGMP: break;
#endif
#if (DBG_UDP > 0)
    case MODULE_UDP:  break;
#endif
#if (DBG_TCP > 0)
    case MODULE_TCP:  break;
#endif
#if (DBG_NBNS > 0)
    case MODULE_NBNS: break;
#endif
#if (DBG_DHCP > 0)
    case MODULE_DHCP: break;
#endif
#if (DBG_DNS > 0)
    case MODULE_DNS:  break;
#endif
#if (DBG_SNMP > 0)
    case MODULE_SNMP: break;
#endif
#if (DBG_BSD > 0)
    case MODULE_BSD:  break;
#endif
#if (DBG_HTTP > 0)
    case MODULE_HTTP: break;
#endif
#if (DBG_FTP > 0)
    case MODULE_FTP:  break;
#endif
#if (DBG_FTPC > 0)
    case MODULE_FTPC: break;
#endif
#if (DBG_TNET > 0)
    case MODULE_TNET: break;
#endif
#if (DBG_TFTP > 0)
    case MODULE_TFTP: break;
#endif
#if (DBG_TFTPC > 0)
    case MODULE_TFTPC:break;
#endif
#if (DBG_SMTP > 0)
    case MODULE_SMTP: break;
#endif
#if (DBG_SNTP > 0)
    case MODULE_SNTP: break;
#endif
    default: return;
  }
#if (DBG_TIME)
  printf (dbg_time());
#endif
  printf ("%s-ERR:",dbg_proc(proc));

  va_start (args,fmt);
  vprintf (fmt,args);
  va_end (args);
}

#endif    /* __NET_DEBUG__ */


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
