/*----------------------------------------------------------------------------
 *      RL-ARM - TCPnet
 *----------------------------------------------------------------------------
 *      Name:    NET_CONFIG.H
 *      Purpose: Common TCPnet Definitions
 *      Rev.:    V4.73
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#ifndef __NET_CONFIG_H__
#define __NET_CONFIG_H__

#include "RTE_Include.h"
#define U32             uint32_t
#define U16             uint16_t
#define U8              uint8_t
/* Definitions */
#define ETH_ADRLEN      6         /* Ethernet Address Length in bytes        */
#define IP_ADRLEN       4         /* IP Address Length in bytes              */
#define OS_HEADER_LEN   4         /* TCPnet 'os_frame' header size           */
                                  /* Frame Header length common for all      */
#define PHY_HEADER_LEN  (2*ETH_ADRLEN + 2) /* network interfaces.            */
#define ETH_MTU         1514      /* Ethernet Frame Max Transfer Unit        */
#define PPP_PROT_IP     0x0021    /* PPP Protocol type: IP                   */
#define TCP_DEF_WINSIZE 4380      /* TCP default window size                 */
#define PASSW_SZ        20        /* Authentication Password Buffer size     */

/* Network Interfaces */
#define NETIF_ETH       0         /* Network interface: Ethernet             */
#define NETIF_PPP       1         /* Network interface: PPP                  */
#define NETIF_SLIP      2         /* Network interface: Slip                 */
#define NETIF_LOCAL     3         /* Network interface: Localhost (loopback) */
#define NETIF_NULL      4         /* Network interface: Null (none)          */

/* Telnet Definitions */
#define TNET_LBUFSZ     96        /* Command Line buffer size (bytes)        */
#define TNET_HISTSZ     128       /* Command History buffer size (bytes)     */
#define TNET_FIFOSZ     128       /* Input character Fifo buffer (bytes)     */

/* SNMP-MIB Definitions */
#define MIB_INTEGER     0x02      /* MIB entry type INTEGER                  */
#define MIB_OCTET_STR   0x04      /* MIB entry type OCTET_STRING             */
#define MIB_OBJECT_ID   0x06      /* MIB entry type OBJECT_IDENTIFIER        */
#define MIB_IP_ADDR     0x40      /* MIB entry type IP ADDRESS (uint8_t[4])       */
#define MIB_COUNTER     0x41      /* MIB entry type COUNTER (uint32_t)            */
#define MIB_GAUGE       0x42      /* MIB entry type GAUGE (uint32_t)              */
#define MIB_TIME_TICKS  0x43      /* MIB entry type TIME_TICKS               */
#define MIB_ATR_RO      0x80      /* MIB entry attribute READ_ONLY           */
#define MIB_OIDSZ       17        /* Max.size of Object ID value             */
#define MIB_STRSZ       110       /* Max.size of Octet String variable       */
#define MIB_READ        0         /* MIB entry Read access                   */
#define MIB_WRITE       1         /* MIB entry Write access                  */

/* SNMP-MIB Macros */
#define MIB_STR(s)      sizeof(s)-1, s
#define MIB_INT(o)      sizeof(o), (void *)&o
#define MIB_IP(ip)      4, (void *)&ip 
#define OID0(f,s)       (f*40 + s) 

/* Debug Module Definitions */
#define MODULE_MEM      0         /* Dynamic Memory Module ID                */
#define MODULE_ETH      1         /* Ethernet Module ID                      */
#define MODULE_PPP      2         /* PPP Module ID                           */
#define MODULE_SLIP     3         /* SLIP Module ID                          */
#define MODULE_ARP      4         /* ARP Module ID                           */
#define MODULE_IP       5         /* IP Module ID                            */
#define MODULE_ICMP     6         /* ICMP Module ID                          */
#define MODULE_IGMP     7         /* IGMP Module ID                          */
#define MODULE_UDP      8         /* UDP Module ID                           */
#define MODULE_TCP      9         /* TCP Module ID                           */
#define MODULE_NBNS     10        /* NBNS Module ID                          */
#define MODULE_DHCP     11        /* DHCP Module ID                          */
#define MODULE_DNS      12        /* DNS Module ID                           */
#define MODULE_SNMP     13        /* SNMP Module ID                          */
#define MODULE_BSD      14        /* BSD Socket Module ID                    */
#define MODULE_HTTP     15        /* HTTP Server Module ID                   */
#define MODULE_FTP      16        /* FTP Server Module ID                    */
#define MODULE_FTPC     17        /* FTP Client Module ID                    */
#define MODULE_TNET     18        /* Telnet Server Module ID                 */
#define MODULE_TFTP     19        /* TFTP Server Module ID                   */
#define MODULE_TFTPC    20        /* TFTP Client Module ID                   */
#define MODULE_SMTP     21        /* SMTP Client Module ID                   */
#define MODULE_SNTP     22        /* SNTP Client Module ID                   */

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C"  {
#endif

typedef struct os_frame {         /* << System frame buffer structure >>     */
  uint16_t length;                     /* Total Length of data in frame           */
  uint16_t index;                      /* Buffer Position Index                   */
  uint8_t  data[1];                    /* Buffer data (protocol headers + data)   */
} OS_FRAME;

typedef struct arp_info {         /* << ARP Cache Entry info >>              */
  uint8_t  State;                      /* ARP Cache entry current state           */
  uint8_t  Type;                       /* Cache Entry type                        */
  uint8_t  Retries;                    /* Number of Retries left                  */
  uint8_t  Tout;                       /* Cache Entry Timeout                     */
  uint8_t  HwAdr[ETH_ADRLEN];          /* Ethernet Hardware Address               */
  uint8_t  IpAdr[IP_ADRLEN];           /* Ethernet IP Address                     */
} ARP_INFO;

typedef struct igmp_info {        /* << IGMP Group info >>                   */
  uint8_t  State;                      /* Group membership current state          */
  uint8_t  Tout;                       /* Timeout Timer for sending reports       */
  uint8_t  Flags;                      /* State machine flags                     */
  uint8_t  GrpIpAdr[IP_ADRLEN];        /* Group IP address                        */
} IGMP_INFO;

typedef struct udp_info {         /* << UDP Socket info >>                   */
  uint8_t  State;                      /* UDP Socket entry current state          */
  uint8_t  Opt;                        /* UDP Socket Options                      */
  uint8_t  Flags;                      /* State machine flags                     */
  uint8_t  Tos;                        /* UDP Type of Service                     */
  uint16_t LocPort;                    /* Local UDP port of Socket                */
  uint8_t  McastTtl;                   /* MultiCast Time To Live                  */
                                  /* Application Event CallBack function     */
  uint16_t (*cb_func)(uint8_t socket, uint8_t *rem_ip, uint16_t port, uint8_t *buf, uint16_t len);
} UDP_INFO;

typedef struct tcp_info {         /* << TCP Socket info >>                   */
  uint8_t  State;                      /* Socket entry current state              */
  uint8_t  Type;                       /* Socket type                             */
  uint8_t  Flags;                      /* State machine flags                     */
  uint8_t  Tos;                        /* Type of service allocated               */
  uint8_t  RemIpAdr[IP_ADRLEN];        /* Remote IP address                       */
  uint16_t RemPort;                    /* Remote TCP port                         */
  uint16_t LocPort;                    /* Local TCP port                          */
  uint16_t MaxSegSize;                 /* Transmit Max. Segment Size              */
  uint16_t Tout;                       /* Socket idle timeout (in seconds)        */
  uint16_t AliveTimer;                 /* Keep Alive timer                        */
  uint16_t RetryTimer;                 /* Retransmission timer                    */
  uint8_t  AckTimer;                   /* Receive Delay-ack timer                 */
  uint8_t  Id;                         /* Socket identification number            */
  uint8_t  Retries;                    /* Number of retries left before aborting  */
  uint8_t  DupAcks;                    /* Number of duplicate acks (fast recovery)*/
  uint32_t SendUna;                    /* Send Sequence Number unacknowledged     */
  uint32_t SendNext;                   /* Next Send Sequence Number               */
  uint32_t SendChk;                    /* Check Sequence Number for dupacks       */
  uint32_t SendWl1;                    /* Sequence Number of last Window update   */
  uint32_t SendWl2;                    /* Acknowledge Number of last Window update*/
  uint16_t SendWin;                    /* Current Send Window                     */
  int16_t RttSa;                      /* Scaled Average for RTT estimator        */
  int16_t RttSv;                      /* Scaled deViation for RTT estimator      */
  uint16_t CWnd;                       /* Congestion Window                       */
  uint16_t SsThresh;                   /* Slow Start Treshold                     */
  uint16_t RecWin;                     /* Current Receive Window                  */
  uint32_t RecNext;                    /* Next Receive Sequence Number            */
  OS_FRAME *unack_list;           /* Unacked queue list                      */
                                  /* Application Event-CallBack function     */
  uint16_t (*cb_func)(uint8_t socket, uint8_t event, uint8_t *p1, uint16_t p2);
} TCP_INFO;

typedef struct bsd_info {         /* << BSD Socket info >>                   */
  uint8_t  State;                      /* BSD Socket current state                */
  uint8_t  Socket;                     /* Binded TCP/UDP Socket                   */
  uint8_t  Flags;                      /* Flags for Socket Option                 */
  uint8_t  Type;                       /* Type of Socket (SOCK_DGRAM/SOCK_STREAM) */
  uint16_t LocPort;                    /* Bind local Port                         */
  uint16_t RemPort;                    /* Remote TCP/UDP Port                     */
  uint32_t LocIP;                      /* Bind local IP address (net byte order)  */
  uint32_t RemIP;                      /* Remote IP address     (net byte order)  */
  uint8_t  AcceptSock;                 /* Accept Socket list                      */
  uint8_t  ParentSock;                 /* Parent (server) socket                  */
  uint8_t  Event;                      /* Wait Event flags in RTX environment     */
  uint8_t  Task;                       /* Socket Owner Task                       */
  uint16_t Tout;                       /* Receive timer value                     */
  void *buf_list;                 /* Rx Data buffer chain                    */
} BSD_INFO;

typedef struct http_info {        /* << HTTP Session info >>                 */
  uint8_t  State;                      /* HTTP Session current state              */
  uint8_t  Socket;                     /* binded TCP Socket                       */
  uint16_t Flags;                      /* State machine Flags                     */
  uint8_t  FType;                      /* File Type: html, jpeg, gif,...          */
  uint8_t  PostSt;                     /* POST method Processing Status           */
  uint16_t DelimSz;                    /* Boundary delimiter size                 */
  uint8_t  UserId;                     /* User Id (authentication enabled)        */
  uint32_t CGIvar;                     /* CGI session private variable (cgi_func) */
  uint32_t DLen;                       /* Data length (number of bytes to send)   */
  uint32_t Count;                      /* Total number of bytes sent              */
  uint16_t BCnt;                       /* Number of bytes sent in last packet     */
  uint8_t  Lang[6];                    /* Selected Web Language: en-us, de, fr .. */
  uint32_t LMDate;                     /* Last Modified Date (UTC format)         */
  uint8_t *Script;                     /* Script Buffer pointer                   */
  uint8_t *pDelim;                     /* Multipart Boundary delimiter value      */
  void *sFile;                    /* Script Interpreter File pointer         */
  void *dFile;                    /* Data File pointer                       */
} HTTP_INFO;

typedef struct http_file {        /* << HTTP Internal File info >>           */
  uint32_t       Id;                   /* File Name Id (CRC32 value of file name) */
  const uint8_t *Start;                /* File Start Address in ROM               */
} const HTTP_FILE;

typedef struct tnet_info {        /* << Telnet Session info >>               */
  uint8_t  State;                      /* Telnet Session current state            */
  uint8_t  Socket;                     /* binded TCP Socket                       */
  uint8_t  Flags;                      /* State machine Flags                     */
  uint8_t  BCnt;                       /* Received Data byte count                */
  uint16_t Tout;                       /* Timeout delay counter                   */
  uint8_t  UserId;                     /* User Id (authentication enabled)        */
  uint8_t  Widx;                       /* Fifo buffer Write index                 */
  uint8_t  Ridx;                       /* Fifo buffer Read index                  */
  uint8_t  hNext;                      /* History command next position           */
  uint8_t  hCurr;                      /* History command current position        */
  uint32_t SVar;                       /* Session private var. (tnet_process_cmd) */
  uint8_t  LBuf[TNET_LBUFSZ];          /* Data Line Buffer                        */
  uint8_t  Fifo[TNET_FIFOSZ];          /* Fifo Buffer for received line/keycodes  */
  uint8_t  Hist[TNET_HISTSZ];          /* Command History buffer                  */
} TNET_INFO;

typedef struct tftp_info {        /* << TFTP Session info >>                 */
  uint8_t  State;                      /* TFTP Session current state              */
  uint8_t  Socket;                     /* binded UDP Data Socket                  */
  uint8_t  Flags;                      /* State machine Flags                     */
  uint8_t  Retries;                    /* Retry counter                           */
  uint8_t  RemIpAdr[IP_ADRLEN];        /* Remote IP address                       */
  uint16_t RemPort;                    /* Remote UDP port (TID)                   */
  uint16_t BlockSz;                    /* Transfer Block size                     */
  uint16_t BlockNr;                    /* Block Number                            */
  uint8_t  Timer;                      /* Timeout Timer value                     */
  uint16_t BufLen;                     /* Length of retransmit buffer             */
  uint8_t  *Buf;                       /* Transmit/Retransmit buffer              */
  void *File;                     /* File Handle pointer                     */
} TFTP_INFO;

typedef struct ftp_info {         /* << FTP Session info >>                  */
  uint8_t  State;                      /* FTP Session current state               */
  uint8_t  Socket;                     /* binded TCP Control Socket               */
  uint16_t Flags;                      /* State machine Flags                     */
  uint8_t  RemIpAdr[IP_ADRLEN];        /* Client IP address                       */
  uint16_t DPort;                      /* TCP Data port (Server or Client)        */
  uint8_t  DSocket;                    /* TCP Data Socket                         */
  uint8_t  UserId;                     /* User Id (authentication enabled)        */
  uint8_t  Resp;                       /* FTP Server Response Code                */
  uint8_t  PathLen;                    /* Size of the Path string                 */
  uint8_t *Path;                       /* Current Working Directory               */
  uint8_t *Name;                       /* Absolute File/Folder Path Name          */
  void *File;                     /* File Handle pointer                     */
} FTP_INFO;

typedef struct dns_cache {        /* << DNS Cache Entry info >>              */
  uint32_t HostId;                     /* Host Id (CRC32 value of host name)      */
  uint32_t Ttl;                        /* Cache Entry Time To Live                */
  uint8_t  IpAdr[IP_ADRLEN];           /* Ethernet IP Address                     */
} DNS_CACHE;

typedef struct localm {           /* << Local Machine info >>                */
  uint8_t  IpAdr[IP_ADRLEN];           /* Local IP Address                        */
  uint8_t  DefGW[IP_ADRLEN];           /* Default GateWay                         */
  uint8_t  NetMask[IP_ADRLEN];         /* Net Mask                                */
  uint8_t  PriDNS[IP_ADRLEN];          /* Primary DNS Server                      */
  uint8_t  SecDNS[IP_ADRLEN];          /* Secondary DNS Server                    */
} LOCALM;

typedef struct remotem {          /* << Remote Machine Info >>               */
  uint8_t  IpAdr[IP_ADRLEN];           /* Remote IP address                       */
  uint8_t  HwAdr[ETH_ADRLEN];          /* Remote Hardware Address                 */
} REMOTEM;

typedef struct mib_entry {        /* << SNMP-MIB Entry Info >>               */
  uint8_t  Type;                       /* Object Type                             */
  uint8_t  OidLen;                     /* Object ID length                        */
  uint8_t  Oid[MIB_OIDSZ];             /* Object ID value                         */
  uint8_t  ValSz;                      /* Size of a Variable                      */
  void *Val;                      /* Pointer to a variable                   */
  void (*cb_func)(int mode);      /* Write/Read event callback function      */
} MIB_ENTRY;

typedef struct sys_cfg {          /* << SYS Configuration info >>            */
  uint32_t *MemPool;                   /* Dynamic Memory Pool buffer              */
  uint32_t MemSize;                    /* Memory Pool size in bytes               */
  uint8_t  TickRate;                   /* Tick Rate in ticks per second           */
  uint8_t  TickItv;                    /* Tick Interval in ms                     */
  uint8_t  T200ms;                     /* Delay 200 ms in ticks                   */
  uint8_t  NetCfg;                     /* Network Interface Configuration flags   */
  uint8_t *HostName;                   /* Local Host Name                         */
} const SYS_CFG;

typedef struct arp_cfg {          /* << ARP Configuration info >>            */
  ARP_INFO *Table;                /* Cache Table array                       */
  uint8_t  TabSize;                    /* Cache Table size                        */
  uint8_t  TimeOut;                    /* Table Entry expiration time in seconds  */
  uint8_t  MaxRetry;                   /* Number of Retries to resolve MAC addr.  */
  uint8_t  Resend;                     /* Resend Timeout in seconds               */
  uint8_t  Notify;                     /* Notify on IP address changes             */
} const ARP_CFG;

typedef struct igmp_cfg {         /* << IGMP Configuration info >>           */
  IGMP_INFO *Table;               /* Group Table array                       */
  uint16_t TabSize;                    /* Group Table size                        */
} const IGMP_CFG;

typedef struct dhcp_cfg {         /* << DHCP Configuration info >>           */
  uint8_t *Vcid;                       /* Vendor Class Identifier                 */
  uint8_t  Opt;                        /* Extra DHCP option flags                 */
} const DHCP_CFG;

typedef struct ppp_cfg {          /* << PPP Configuration info >>            */
  uint32_t ACCmap;                     /* Async Control Character map             */
  uint16_t SerTout;                    /* Serial Frame Receive Timeout            */
  uint16_t RetryTout;                  /* Retry Timeout in ticks                  */
  uint16_t EchoTout;                   /* LCP Echo Timeout in seconds             */
  uint8_t  MaxRetry;                   /* Number of Retries                       */
  uint8_t  EnAuth;                     /* Enabled Authentication protocols        */
} const PPP_CFG;

typedef struct udp_cfg {          /* << UDP Configuration info >>            */
  UDP_INFO *Scb;                  /* Socket Control Block array              */
  uint8_t  NumSocks;                   /* Number of UDP Sockets                   */
} const UDP_CFG;

typedef struct tcp_cfg {          /* << TCP Configuration info >>            */
  TCP_INFO *Scb;                  /* Socket Control Block array              */
  uint8_t  NumSocks;                   /* Number of TCP Sockets                   */
  uint8_t  MaxRetry;                   /* Number of Retries                       */
  uint16_t RetryTout;                  /* Retry Timeout in ticks                  */
  uint16_t T2MSLTout;                  /* Maximum Segment Life Time               */
  uint16_t SynRetryTout;               /* SYN Retry Timeout in ticks              */
  uint16_t InitRetryTout;              /* Initial Retransmit timeout in ticks     */
  uint16_t DefTout;                    /* Default Connect Timeout in seconds      */
  uint16_t MaxSegSize;                 /* Maximum Segment Size value              */
  uint16_t RecWinSize;                 /* Receiving Window Size in bytes          */
  uint8_t  ConRetry;                   /* Number of Retries to Connect            */
} const TCP_CFG;

typedef struct http_cfg {         /* << HTTP Configuration info >>           */
  HTTP_INFO *Scb;                 /* Session Control Block array             */
  uint8_t  NumSess;                    /* Max. Number of Active Sessions          */
  uint8_t  EnAuth;                     /* Enable User Authentication              */
  uint16_t PortNum;                    /* Listening Port number                   */
  uint8_t const *SrvId;                /* Server-Id http header                   */
  uint8_t const *Realm;                /* Authentication Realm                    */
  uint8_t const *User;                 /* Authentication User name                */
  uint8_t *Passw;                      /* Authentication Password                 */
} const HTTP_CFG;

typedef struct tnet_cfg {         /* << TNET Configuration info >>           */
  TNET_INFO *Scb;                 /* Session Control Block array             */
  uint8_t  NumSess;                    /* Max. Number of Active Sessions          */
  uint8_t  EnAuth;                     /* Enable User Authentication              */
  uint8_t  NoEcho;                     /* Disable Server Echo mode                */
  uint16_t PortNum;                    /* Listening Port number                   */
  uint16_t IdleTout;                   /* Idle Connection timeout in ticks        */
  uint8_t const *User;                 /* Authentication User Name                */
  uint8_t *Passw;                      /* Authentication Password                 */
} const TNET_CFG;

typedef struct tftp_cfg {         /* << TFTP Configuration info >>           */
  TFTP_INFO *Scb;                 /* Session Control Block array             */
  uint8_t  NumSess;                    /* Max. Number of Active Sessions          */
  uint8_t  MaxRetry;                   /* Number of Retries                       */
  uint16_t PortNum;                    /* Listening Port number                   */
  uint8_t  DefTout;                    /* Inactive Session Timeout in seconds     */
  uint8_t  EnFwall;                    /* Enable Firewall Support                 */
} const TFTP_CFG;

typedef struct tftpc_cfg {        /* << TFTPC Configuration info >>          */
  uint16_t BlockSize;                  /* Transfer Block size                     */
  uint16_t RetryTout;                  /* Retry Timeout in ticks                  */
  uint8_t  MaxRetry;                   /* Number of Retries                       */
} const TFTPC_CFG;

typedef struct ftp_cfg {          /* << FTP Configuration info >>            */
  FTP_INFO *Scb;                  /* Session Control Block array             */
  uint8_t  NumSess;                    /* Max. Number of Active Sessions          */
  uint8_t  EnAuth;                     /* Enable User Authentication              */
  uint16_t PortNum;                    /* Listening Port number                   */
  uint16_t IdleTout;                   /* Idle Connection timeout in ticks        */
  uint8_t  MsgLen;                     /* Length of welcome message               */
  uint8_t const *Msg;                  /* Server Welcome message                  */ 
  uint8_t const *User;                 /* Authentication User Name                */
  uint8_t *Passw;                      /* Authentication Password                 */
} const FTP_CFG;

typedef struct ftpc_cfg {         /* << FTPC Configuration info >>           */
  uint8_t  DefTout;                    /* Default inactivity timeout              */
  uint8_t  PasvMode;                   /* FTP Passive Mode used                   */
} const FTPC_CFG;

typedef struct dns_cfg {          /* << DNS Configuration info >>            */
  DNS_CACHE *Table;               /* Cache Table array                       */
  uint8_t  TabSize;                    /* Cache Table size                        */
} const DNS_CFG;

typedef struct snmp_cfg {         /* << SNMP Configuration info >>           */
  uint16_t PortNum;                    /* Listening Port number                   */
  uint16_t TrapPort;                   /* Trap Port number                        */
  uint8_t  TrapIp[IP_ADRLEN];          /* Trap Server IP Address                  */
  uint8_t const *Community;            /* Community Name                          */
  uint8_t  TickItv;                    /* Tick Interval in ticks                  */
} const SNMP_CFG;

typedef struct sntp_cfg {         /* << SNTP Configuration info >>           */
  uint8_t  SrvIp[IP_ADRLEN];           /* NTP/SNTP Server IP Address              */
  uint8_t  Bcast;                      /* Broadcast client mode (for LAN only)    */
} const SNTP_CFG;

typedef struct bsd_cfg {          /* << BSD Configuration info >>            */
  BSD_INFO *Scb;                  /* Socket Control Block array              */
  uint8_t  NumSocks;                   /* Number of BSD Sockets                   */
  uint8_t  InRtx;                      /* Running in RTX environment              */
  uint16_t RcvTout;                    /* Blocking recv timeout in ticks          */
} const BSD_CFG;

typedef enum {                    /* << Fatal System Error Codes >>          */
  ERR_MEM_ALLOC,                  /* Alloc memory failed, out of memory      */
  ERR_MEM_FREE,                   /* Free memory failed, memory slot invalid */
  ERR_MEM_CORRUPT,                /* Memory corruption detected              */
  ERR_MEM_LOCK,                   /* Locked memory function re-entered error */
  ERR_UDP_ALLOC,                  /* No free UDP sockets available           */
  ERR_TCP_ALLOC,                  /* No free TCP sockets available           */
  ERR_TCP_STATE                   /* TCP socket in undefined state           */
} ERROR_CODE;

typedef struct {                        /* RL Time format (FFS, TCPnet)      */
  uint8_t  hr;                               /* Hours    [0..23]                  */
  uint8_t  min;                              /* Minutes  [0..59]                  */
  uint8_t  sec;                              /* Seconds  [0..59]                  */
  uint8_t  day;                              /* Day      [1..31]                  */
  uint8_t  mon;                              /* Month    [1..12]                  */
  uint16_t year;                             /* Year     [1980..2107]             */
} RL_TIME;
/*----------------------------------------------------------------------------
 *      Functions
 *---------------------------------------------------------------------------*/

/* Net_Config.c */
extern void init_system (void);
extern void run_system (void);
extern void sys_error (ERROR_CODE code);

/* at_Mem.c */
extern OS_FRAME *alloc_mem (uint32_t byte_size);
extern void free_mem (OS_FRAME *mem_ptr);

/* at_Ethernet.c */
extern void eth_init_link (void);
extern void eth_run_link (void);
extern void put_in_queue (OS_FRAME *frame);
extern bool eth_send_frame (OS_FRAME *frame);
extern bool eth_chk_adr (OS_FRAME *frame);
extern uint8_t  *eth_get_adr (uint8_t *ipadr);

/* at_Ppp.c */
extern void ppp_init_link (void);
extern void ppp_run_link (void);
extern bool ppp_send_frame (OS_FRAME *frame, uint16_t prot);

/* at_Ppp_Pap.c */
extern void pap_init (void);
extern void pap_process (OS_FRAME *frame);
extern void pap_run (void);

/* at_Ppp_Chap.c */
extern void chap_init (void);
extern void chap_process (OS_FRAME *frame);
extern void chap_run (void);

/* at_Slip.c */
extern void slip_init_link (void);
extern void slip_run_link (void);
extern bool slip_send_frame (OS_FRAME *frame);

/* at_Lib.c */
extern int  mem_copy (void *dp, void *sp, int len);
extern void mem_rcopy (void *dp, void *sp, int len);
extern bool mem_comp (void *sp1, void *sp2, int len);
extern void mem_set (void *dp, uint8_t val, int len);
extern bool mem_test (void *sp, uint8_t val, int len);
extern bool str_scomp (uint8_t *sp, uint8_t const *cp);
extern int  str_copy (uint8_t *dp, uint8_t *sp);
extern void str_up_case (uint8_t *dp, uint8_t *sp);

/* at_Arp.c */
extern void arp_notify (void);
extern bool arp_get_info (REMOTEM *info);

/* at_Ip.c */
extern void ip_init (void);
extern void ip_run_local (void);

/* at_Dhcp.c */
extern void dhcp_cbfunc (uint8_t opt, uint8_t *val, uint16_t len);

/* at_Icmp.c */
extern void icmp_init (void);
extern void icmp_run_engine (void);

/* at_Igmp.c */
extern void igmp_init (void);
extern void igmp_run_host (void);
extern void igmp_process (OS_FRAME *frame);

/* at_Udp.c */
extern void udp_init (void);
extern void udp_process (OS_FRAME *frame);

/* at_Tcp.c */
extern void tcp_init (void);
extern void tcp_poll_sockets (void);
extern void tcp_process (OS_FRAME *frame);

/* at_Bsd.c */
extern void bsd_init (void);
extern void bsd_poll_sockets (void);
extern uint8_t   bsd_wait (BSD_INFO *bsd_s, uint8_t evt);
extern void bsd_enable (BSD_INFO *bsd_s, uint8_t evt);

/* at_Bsd_Host.c */
extern void bsd_init_host (void);

/* at_Http.c */
extern void http_init (void);
extern void http_run_server (void);
extern void *http_fopen (uint8_t *name);
extern void http_fclose (void *file);
extern uint16_t  http_fread (void *file, uint8_t *buf, uint16_t len);
extern bool http_fgets (void *file, uint8_t *buf, uint16_t size);
extern uint32_t  http_finfo (uint8_t *name);
extern void cgi_process_var (uint8_t *qstr);
extern void cgi_process_data (uint8_t code, uint8_t *dat, uint16_t len);
extern uint16_t  cgi_func (uint8_t *env, uint8_t *buf, uint16_t buflen, uint32_t *pcgi);
extern uint8_t  *cgx_content_type (void);
extern uint8_t  *http_encoding (void);
extern bool http_accept_host (uint8_t *rem_ip, uint16_t rem_port);
extern bool http_file_access (uint8_t *fname, uint8_t user_id);
extern uint8_t   http_check_account (uint8_t *user, uint8_t *passw);
extern uint8_t  *http_get_var (uint8_t *env, void *ansi, uint16_t maxlen);
extern uint8_t  *http_get_lang (void);
extern void http_get_info (REMOTEM *info);
extern uint8_t   http_get_session (void);
extern uint8_t   http_get_user_id (void);
extern uint8_t  *http_get_content_type (void);
extern uint32_t  http_date (RL_TIME *time);

/* at_Telnet.c */
extern void tnet_init (void);
extern void tnet_run_server (void);
extern uint16_t  tnet_cbfunc (uint8_t code, uint8_t *buf, uint16_t buflen);
extern uint16_t  tnet_process_cmd (uint8_t *cmd, uint8_t *buf, uint16_t buflen, uint32_t *pvar);
extern bool tnet_ccmp (uint8_t *buf, uint8_t *cmd);
extern void tnet_set_delay (uint16_t dly);
extern void tnet_get_info (REMOTEM *info);
extern uint8_t   tnet_get_session (void);
extern uint8_t   tnet_get_user_id (void);
extern bool tnet_msg_poll (uint8_t session);
extern bool tnet_accept_host (uint8_t *rem_ip, uint16_t rem_port);
extern uint8_t   tnet_check_account (uint8_t code, uint8_t *id);

/* at_Tftp.c */
extern void tftp_init (void);
extern void tftp_run_server (void);
extern void *tftp_fopen (uint8_t *fname, uint8_t *mode);
extern void tftp_fclose (void *file);
extern uint16_t  tftp_fread (void *file, uint8_t *buf, uint16_t len);
extern uint16_t  tftp_fwrite (void *file, uint8_t *buf, uint16_t len);
extern bool tftp_accept_host (uint8_t *rem_ip, uint16_t rem_port);

/* at_Tftpc.c */
extern void tftpc_init (void);
extern void tftpc_run_client (void);
extern void *tftpc_fopen (uint8_t *fname, uint8_t *mode);
extern void tftpc_fclose (void *file);
extern uint16_t  tftpc_fread (void *file, uint8_t *buf, uint16_t len);
extern uint16_t  tftpc_fwrite (void *file, uint8_t *buf, uint16_t len);

/* at_Ftp.c */
extern void ftp_init (void);
extern void ftp_run_server (void);
extern void *ftp_fopen (uint8_t *fname, uint8_t *mode);
extern void ftp_fclose (void *file);
extern uint16_t  ftp_fread (void *file, uint8_t *buf, uint16_t len);
extern uint16_t  ftp_fwrite (void *file, uint8_t *buf, uint16_t len);
extern bool ftp_fdelete (uint8_t *fname);
extern bool ftp_frename (uint8_t *fname, uint8_t *newn);
extern uint16_t  ftp_ffind (uint8_t code, uint8_t *buf, uint8_t *mask, uint16_t len);
extern bool ftp_accept_host (uint8_t *rem_ip, uint16_t rem_port);
extern uint8_t   ftp_check_account (uint8_t code, uint8_t *id);
extern uint8_t   ftp_get_user_id (void);
extern bool ftp_file_access (uint8_t *fname, uint8_t mode, uint8_t user_id);
extern void ftp_evt_notify (uint8_t evt);

/* at_Ftpc.c */
extern void ftpc_init (void);
extern void ftpc_run_client (void);
extern void *ftpc_fopen (uint8_t *mode);
extern void ftpc_fclose (void *file);
extern uint16_t  ftpc_fread (void *file, uint8_t *buf, uint16_t len);
extern uint16_t  ftpc_fwrite (void *file, uint8_t *buf, uint16_t len);
extern uint16_t  ftpc_cbfunc (uint8_t code, uint8_t *buf, uint16_t buflen);

/* at_Dhcp.c */
extern void dhcp_init (void);
extern void dhcp_run_client (void);
#define init_dhcp   dhcp_init

/* at_Nbns.c */
extern void nbns_init (void);

/* at_Dns.c */
extern void dns_init (void);
extern void dns_run_client (void);
extern uint8_t   get_host_by_name (uint8_t *hostn, void (*cbfunc)(uint8_t, uint8_t *));

/* at_Smtp.c */
extern void smtp_init (void);
extern void smtp_run_client (void);
extern uint16_t  smtp_cbfunc (uint8_t code, uint8_t *buf, uint16_t buflen, uint32_t *pvar);
extern bool smtp_accept_auth (uint8_t *srv_ip);

/* at_Snmp.c */
extern void snmp_init (void);
extern void snmp_run_agent (void);

/* at_Sntp.c */
extern void sntp_init (void);
extern void sntp_run_client (void);

/* Ethernet Device Driver */
extern void init_ethernet (void);
extern void send_frame (OS_FRAME *frame);
__weak void poll_ethernet (void);
extern void int_enable_eth (void);
extern void int_disable_eth (void);

/* Serial Device Driver */
extern void init_serial (void);
extern int  com_getchar (void);
extern bool com_putchar (uint8_t c);
extern bool com_tx_active (void);

/* Modem Device Driver */
extern void modem_init (void);
extern void modem_dial (uint8_t *dialnum);
extern void modem_hangup (void);
extern void modem_listen (void);
extern bool modem_online (void);
extern bool modem_process (uint8_t ch);
extern void modem_run (void);


/*----------------------------------------------------------------------------
 *                             TCPnet API
 *---------------------------------------------------------------------------*/
#define __used          __attribute__((used))
/* UDP Options */
#define UDP_OPT_SEND_CS    0x01   /* Calculate Checksum for UDP send frames  */
#define UDP_OPT_CHK_CS     0x02   /* Verify Checksum for received UDP frames */

/* TCP Socket Types */
#define TCP_TYPE_SERVER    0x01   /* Socket Type Server (open for listening) */
#define TCP_TYPE_CLIENT    0x02   /* Socket Type Client (initiate connect)   */
#define TCP_TYPE_DELAY_ACK 0x04   /* Socket Type Delayed Acknowledge         */
#define TCP_TYPE_FLOW_CTRL 0x08   /* Socket Type Flow Control                */
#define TCP_TYPE_KEEP_ALIVE 0x10  /* Socket Type Keep Alive                  */
#define TCP_TYPE_CLIENT_SERVER (TCP_TYPE_SERVER | TCP_TYPE_CLIENT)

/* TCP Callback Events */
#define TCP_EVT_CONREQ     0      /* Connect request received event          */
#define TCP_EVT_CONNECT    1      /* Connection established event            */
#define TCP_EVT_CLOSE      2      /* Connection was properly closed          */
#define TCP_EVT_ABORT      3      /* Connection is for some reason aborted   */
#define TCP_EVT_ACK        4      /* Previously send data acknowledged       */
#define TCP_EVT_DATA       5      /* Data received event                     */

/* TCP States */
#define TCP_STATE_FREE     0      /* Entry is free and unused                */
#define TCP_STATE_CLOSED   1      /* Entry allocated, socket still closed    */
#define TCP_STATE_LISTEN   2      /* Socket waiting for incoming connection  */
#define TCP_STATE_SYN_REC  3      /* SYN frame received                      */
#define TCP_STATE_SYN_SENT 4      /* SYN packet sent to establish a connect. */
#define TCP_STATE_FINW1    5      /* Tcp_close started FIN packet was sent   */
#define TCP_STATE_FINW2    6      /* Our FIN ack-ed, waiting for remote FIN  */
#define TCP_STATE_CLOSING  7      /* Received FIN independently of our FIN   */
#define TCP_STATE_LAST_ACK 8      /* Waiting for last ACK for our FIN        */
#define TCP_STATE_TWAIT    9      /* Timed waiting for 2MSL                  */
#define TCP_STATE_CONNECT  10     /* TCP Connection established              */

/* BSD Socket Address Family */
#define AF_UNSPEC          0      /* Unspecified                             */
#define AF_INET            1      /* Internet Address Family (UDP, TCP)      */
#define AF_NETBIOS         2      /* NetBios-style addresses                 */

/* BSD Protocol families, same as address families */
#define PF_UNSPEC          AF_UNSPEC
#define PF_INET            AF_INET
#define PF_NETBIOS         AF_NETBIOS

/* BSD Socket Type */
#define SOCK_STREAM        1      /* Stream Socket (Connection oriented)     */
#define SOCK_DGRAM         2      /* Datagram Socket (Connectionless)        */

/* BSD Socket Protocol */
#define IPPROTO_TCP        1      /* TCP Protocol                            */
#define IPPROTO_UDP        2      /* UDP Protocol                            */

/* BSD Internet Addresses */
#define INADDR_ANY     0x00000000 /* All IP addresses accepted               */
#define INADDR_NONE    0xffffffff /* No IP address accepted                  */

/* BSD Socket Return values */
#define SCK_SUCCESS         0     /* Success                                 */
#define SCK_ERROR         (-1)    /* General Error                           */
#define SCK_EINVALID      (-2)    /* Invalid socket descriptor               */
#define SCK_EINVALIDPARA  (-3)    /* Invalid parameter                       */
#define SCK_EWOULDBLOCK   (-4)    /* It would have blocked.                  */
#define SCK_EMEMNOTAVAIL  (-5)    /* Not enough memory in memory pool        */
#define SCK_ECLOSED       (-6)    /* Connection is closed or aborted         */
#define SCK_ELOCKED       (-7)    /* Socket is locked in RTX environment     */
#define SCK_ETIMEOUT      (-8)    /* Socket, Host Resolver timeout           */
#define SCK_EINPROGRESS   (-9)    /* Host Name resolving in progress         */
#define SCK_ENONAME       (-10)   /* Host Name not existing                  */

/* BSD Socket flags parameter */
#define MSG_DONTWAIT       0x01   /* Enables non-blocking operation          */
#define MSG_PEEK           0x02   /* Peeks at the incoming data              */

/* BSD Socket ioctl commands */
#define FIONBIO            1      /* Set mode (blocking/non-blocking)        */
#define FIO_DELAY_ACK      2      /* Set DELAY_ACK mode for stream socket    */
#define FIO_KEEP_ALIVE     3      /* Set KEEP_ALIVE mode for stream socket   */
#define FIO_FLOW_CTRL      4      /* Set FLOW_CTRL mode for stream socket    */

/* ICMP (ping) Callback Events */
#define ICMP_EVT_SUCCESS   0      /* Pinged Host responded                   */
#define ICMP_EVT_TIMEOUT   1      /* Timeout, no ping response received      */

/* DNS Client Callback Events */
#define DNS_EVT_SUCCESS    0      /* Host name successfully resolved         */
#define DNS_EVT_NONAME     1      /* DNS Error, no such name                 */
#define DNS_EVT_TIMEOUT    2      /* Timeout resolving host                  */
#define DNS_EVT_ERROR      3      /* Erroneous response packet               */

/* DNS 'get_host_by_name()' result codes */
#define DNS_RES_OK         0      /* Resolver successfully started           */
#define DNS_ERROR_BUSY     1      /* Resolver busy, can't process request    */
#define DNS_ERROR_LABEL    2      /* Label in Hostname not valid             */
#define DNS_ERROR_NAME     3      /* Entire Hostname not valid               */
#define DNS_ERROR_NOSRV    4      /* Prim. DNS server not specified (0.0.0.0)*/
#define DNS_ERROR_PARAM    5      /* Invalid parameter                       */

/* SMTP Client Callback Events */
#define SMTP_EVT_SUCCESS   0      /* Email successfully sent                 */
#define SMTP_EVT_TIMEOUT   1      /* Timeout sending email                   */
#define SMTP_EVT_ERROR     2      /* Error when sending email                */

/* FTP Client Commands */
#define FTPC_CMD_PUT       0      /* Puts a file on FTP server               */
#define FTPC_CMD_GET       1      /* Retrieves a file from FTP server        */
#define FTPC_CMD_APPEND    2      /* Append file on FTP server (with create) */
#define FTPC_CMD_DELETE    3      /* Deletes a file on FTP server            */
#define FTPC_CMD_LIST      4      /* Lists files stored on FTP server        */
#define FTPC_CMD_RENAME    5      /* Renames a file on FTP server            */
#define FTPC_CMD_MKDIR     6      /* Makes a directory on FTP server         */
#define FTPC_CMD_RMDIR     7      /* Removes an empty directory on FTP server*/
#define FTPC_CMD_NLIST     8      /* Lists file names only (short format)    */

/* FTP Client Callback Events */
#define FTPC_EVT_SUCCESS   0      /* File operation successful               */
#define FTPC_EVT_TIMEOUT   1      /* Timeout on file operation               */
#define FTPC_EVT_LOGINFAIL 2      /* Login error, username/passw invalid     */
#define FTPC_EVT_NOACCESS  3      /* File access not allowed                 */
#define FTPC_EVT_NOTFOUND  4      /* File not found                          */
#define FTPC_EVT_NOPATH    5      /* Working directory path not found        */
#define FTPC_EVT_ERRLOCAL  6      /* Local file open error                   */
#define FTPC_EVT_ERROR     7      /* Generic FTP client error                */

/* TFTP Client Callback Events */
#define TFTPC_EVT_SUCCESS  0      /* File operation successful               */
#define TFTPC_EVT_TIMEOUT  1      /* Timeout on file operation               */
#define TFTPC_EVT_NOACCESS 2      /* File access not allowed                 */
#define TFTPC_EVT_NOTFOUND 3      /* File not found                          */
#define TFTPC_EVT_DISKFULL 4      /* Disk full (local or remote)             */
#define TFTPC_EVT_ERROR    5      /* Generic TFTP client error               */

/* FTP Server Notification events */
#define FTP_EVT_LOGIN      0      /* User logged in, session is busy         */
#define FTP_EVT_LOGOUT     1      /* User logged out, session is idle        */
#define FTP_EVT_LOGFAIL    2      /* User login failed (invalid credentials) */
#define FTP_EVT_DOWNLOAD   3      /* File download ended                     */
#define FTP_EVT_UPLOAD     4      /* File upload ended                       */
#define FTP_EVT_DELETE     5      /* File deleted                            */
#define FTP_EVT_RENAME     6      /* File or directory renamed               */
#define FTP_EVT_MKDIR      7      /* Directory created                       */
#define FTP_EVT_RMDIR      8      /* Directory removed                       */
#define FTP_EVT_ERRLOCAL   9      /* Local file operation error              */
#define FTP_EVT_DENIED     10     /* Requested file operation denied         */
#define FTP_EVT_ERROR      11     /* Generic file operation error            */

/* ARP Cache Entry types */
#define ARP_FIXED_IP       0      /* Fixed IP adrs is refreshed after tout   */
#define ARP_TEMP_IP        1      /* Temp adrs is removed after timeout      */

/* BSD Socket typedef's */
typedef struct sockaddr {         /* << Generic Socket Address structure >>  */
  uint16_t  sa_family;                 /* Address family                          */
  char sa_data[14];               /* Direct address (up to 14 bytes)         */
} SOCKADDR;

#pragma push
#pragma anon_unions

typedef struct in_addr {          /* << Generic IPv4 Address structure >>    */
  union {
    struct {
      uint8_t s_b1,s_b2,s_b3,s_b4;     /* IP address, byte access                 */
    };
    struct {
      uint16_t s_w1,s_w2;              /* IP address, short int access            */
    };
    uint32_t s_addr;                   /* IP address in network byte order        */
  };
} IN_ADDR;
#pragma pop

typedef struct sockaddr_in {      /* << IPv4 Socket Address structure >>     */
  int16_t sin_family;                 /* Socket domain                           */
  uint16_t sin_port;                   /* Port                                    */
  IN_ADDR sin_addr;               /* IP address                              */
  int8_t  sin_zero[8];                /* reserved                                */
} SOCKADDR_IN;

typedef struct hostent {          /* << BSD Host Entry structure >>          */
  char *h_name;                   /* Official name of host                   */
  char **h_aliases;               /* Pointer to an array of alias names      */
  int16_t  h_addrtype;                /* Address Type: AF_INET, AF_NETBIOS       */
  int16_t  h_length;                  /* Length of address in bytes              */
  char **h_addr_list;             /* Pointer to an array of IPv4 addresses   */
} HOSTENT;

extern void init_TcpNet (void);
extern bool main_TcpNet (void);
extern void timer_tick (void);
extern uint8_t   udp_get_socket (uint8_t tos, uint8_t opt, 
                            uint16_t (*listener)(uint8_t socket, uint8_t *remip, uint16_t port, uint8_t *buf, uint16_t len));
extern bool udp_release_socket (uint8_t socket);
extern bool udp_open (uint8_t socket, uint16_t locport);
extern bool udp_close (uint8_t socket);
extern bool udp_mcast_ttl (uint8_t socket, uint8_t ttl);
extern uint8_t  *udp_get_buf (uint16_t size);
extern bool udp_send (uint8_t socket, uint8_t *remip, uint16_t remport, uint8_t *buf, uint16_t dlen);
extern uint8_t   tcp_get_socket (uint8_t type, uint8_t tos, uint16_t tout,
                            uint16_t (*listener)(uint8_t socket, uint8_t event, uint8_t *buf, uint16_t len));
extern bool tcp_release_socket (uint8_t socket);
extern bool tcp_listen (uint8_t socket, uint16_t locport);
extern bool tcp_connect (uint8_t socket, uint8_t *remip, uint16_t remport, uint16_t locport);
extern uint8_t  *tcp_get_buf (uint16_t size);
extern uint16_t  tcp_max_dsize (uint8_t socket);
extern bool tcp_check_send (uint8_t socket);
extern uint8_t   tcp_get_state (uint8_t socket);
extern bool tcp_send (uint8_t socket, uint8_t *buf, uint16_t dlen);
extern bool tcp_close (uint8_t socket);
extern bool tcp_abort (uint8_t socket);
extern void tcp_reset_window (uint8_t socket);
extern bool arp_cache_ip (uint8_t *ipadr, uint8_t type);
extern bool arp_cache_mac (uint8_t *hwadr);
extern void ppp_listen (const char *user, const char *passw);
extern void ppp_connect (const char *dialnum, const char *user, const char *passw);
extern void ppp_close (void);
extern bool ppp_is_up (void);
extern void slip_listen (void);
extern void slip_connect (const char *dialnum);
extern void slip_close (void);
extern bool slip_is_up (void);
extern uint8_t   get_host_by_name (uint8_t *hostn, void (*cbfunc)(uint8_t event, uint8_t *host_ip));
extern bool smtp_connect (uint8_t *ipadr, uint16_t port, void (*cbfunc)(uint8_t event));
extern void dhcp_disable (void);
extern bool igmp_join (uint8_t *group_ip);
extern bool igmp_leave (uint8_t *group_ip);
extern bool snmp_trap (uint8_t *manager_ip, uint8_t gen_trap, uint8_t spec_trap, uint16_t *obj_list);
extern bool snmp_set_community (const char *community);
extern bool icmp_ping (uint8_t *remip, void (*cbfunc)(uint8_t event));
extern bool ftpc_connect (uint8_t *ipadr, uint16_t port, uint8_t command, void (*cbfunc)(uint8_t event));
extern bool tftpc_put (uint8_t *ipadr, uint16_t port,
                       const char *src, const char *dst, void (*cbfunc)(uint8_t event));
extern bool tftpc_get (uint8_t *ipadr, uint16_t port, 
                       const char *src, const char *dst, void (*cbfunc)(uint8_t event));
extern bool sntp_get_time (uint8_t *ipadr, void (*cbfunc)(uint32_t utc_time));
extern void ftp_evt_notify (uint8_t evt);

/* BSD Socket API */
extern int  socket (int family, int type, int protocol);
extern int  bind (int sock, const SOCKADDR *addr, int addrlen);
extern int  listen (int sock, int backlog);
extern int  accept (int sock, SOCKADDR *addr, int *addrlen);
extern int  connect (int sock, SOCKADDR *addr, int addrlen);
extern int  send (int sock, const char *buf, int len, int flags);
extern int  sendto (int sock, const char *buf, int len, int flags, SOCKADDR *to, int tolen);
extern int  recv (int sock, char *buf, int len, int flags);
extern int  recvfrom (int sock, char *buf, int len, int flags, SOCKADDR *from, int *fromlen);
extern int  closesocket (int sock);
extern int  getpeername (int sock, SOCKADDR *name, int *namelen);
extern int  getsockname (int sock, SOCKADDR *name, int *namelen);
extern int  ioctlsocket (int sock, long cmd, unsigned long *argp);
extern HOSTENT *gethostbyname (const char *name, int *err);
#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

#endif

