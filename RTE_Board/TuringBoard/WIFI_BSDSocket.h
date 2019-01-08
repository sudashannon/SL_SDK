#ifndef __WIFI_BSD_SOCKET_H
#define __WIFI_BSD_SOCKET_H
typedef uint32_t   socklen_t; 
#define AF_INET                 (2) 
#define PF_INET                 (AF_INET)
#define AF_INET6                (3)  
#define PF_INET6                (AF_INET6)
//
// Socket types
//
#define	SOCK_STREAM             (1)  /* TCP */
#define	SOCK_DGRAM              (2)  /* UDP */
#define	SOCK_RAW                (3)  /* IP  */
typedef  struct sockaddr_in {
	uint16_t sin_port ;		//Port number
	uint16_t sin_family ;	        //ATH_AF_INET
	uint32_t sin_addr ;		//IPv4 Address
}SOCKADDR_IN;
typedef  struct in6_addr {
   uint8_t   addr[16];             /* 128 bit IPv6 address */
} IN6_ADDR_T;
typedef struct sockaddr_in6
{
   uint16_t        sin6_family ;   // ATH_AF_INET6 
   uint16_t        sin6_port ;     // transport layer port #
   uint32_t        sin6_flowinfo ; // IPv6 flow information 
   IN6_ADDR_T      sin6_addr ;      // IPv6 address 
   uint32_t        sin6_scope_id ; // set of interfaces for a scope 
} SOCKADDR_IN6;
#ifdef htonl
#undef htonl
#endif /* htonl */
#ifdef ntohl
#undef ntohl
#endif /* ntohl */
#ifdef htons
#undef htons
#endif /* htons */
#ifdef ntohs
#undef ntohs
#endif /* ntohs */
#define htons(n) 				(((n & 0xff) << 8) | ((n & 0xff00) >> 8))
#define htonl(n) 				(((n & 0xff) << 24) |\
			    		     	((n & 0xff00) << 8) |\
			   			        ((n & 0xff0000UL) >> 8) |\
			    		    	((n & 0xff000000UL) >> 24))
#define ntohs(n) 				htons(n)
#define ntohl(n) 				htonl(n)
int socket(int domain, int type, int protocol);
int bind(int sockfd,const void *myaddr, socklen_t addrlen);
int connect(int sockfd, void *serv_addr, int addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, void *addr, int *addrlen);
int send(int sockfd, const void *msg, int len, int flags);
int sendto(int sockfd, const void *data, size_t size, int flags, const void *to, socklen_t tolen);
int recv(int sockfd, void *buf, int len, unsigned int flags);
int recvfrom(int sockfd, void *mem, size_t len, int flags, void *from, socklen_t *fromlen);
int shutdown(int sockfd, int how);
int close(int sockfd);
int gethostbyname(const char *hostname ,uint8_t namelen ,uint32_t* out_ipaddr,uint16_t family);
int select(int sockfd, uint32_t timeout_ms);
#endif
