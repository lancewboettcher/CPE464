/* 
 * CPE 464
 * Program 1 - Trace
 * Lance Boettcher
 */

#define TYPE_IP 8
#define TYPE_ARP 1544

#define ETHERNET_LENGTH 14
#define IP_LENGTH 20
#define TCP_LENGTH 20
#define TCP_PSEUDO_LENGTH 12

#define IP_VERSION_LOC 4
#define DSCP_LOC 2

#define ICMP_CODE 1
#define TCP_CODE 6
#define UDP_CODE 17

#define TCP_OFFSET_LOC 4
#define ACK_FLAG_LOC 4
#define TCP_CHECKSUM_SIZE 16384

#define ICMP_REQUEST 8
#define ICMP_REPLY 0
#define BAD_IP 11
#define BAD_IP_TYPE 109

#define ARP_REQUEST 256
#define ARP_REPLY 512

#define DNS 53
#define HTTP 80
#define Telnet 23
#define FTP 21
#define POP3 110
#define SMTP 25

struct ethernet {

   unsigned char dest[6];
   unsigned char src[6];
   uint16_t type;

}__attribute__((packed));

struct ip {

   unsigned char versionAndLength;
   unsigned char tos;
   uint16_t size;
   uint16_t id;
   uint16_t fragOffset;
   unsigned char ttl;
   unsigned char protocol;
   uint16_t checksum;
   struct in_addr src;
   struct in_addr dest;
   uint32_t options;

}__attribute__((packed));

struct arp {

   uint16_t hardwareType;
   uint16_t protocolType;
   unsigned char hardwareAddressLength;
   unsigned char protocolAddressLength;
   uint16_t opcode;
   struct ether_addr senderMAC;
   struct in_addr senderIP;
   struct ether_addr targetMAC;
   struct in_addr targetIP;

}__attribute__((packed));

struct icmp {

   unsigned char type;
   unsigned char code;
   uint16_t checksum;
   uint32_t restOfHeader;

}__attribute__((packed));

struct tcp {

   uint16_t src;
   uint16_t dest;
   uint32_t sequenceNumber;
   uint32_t ackNumber;
   unsigned char offsetAndReserved;
   unsigned char flags;
   uint16_t windowSize;
   uint16_t checksum; 
   uint16_t urgent; 
   uint32_t padding;

}__attribute__((packed));

struct tcpPseudo {

   struct in_addr ipSrc;
   struct in_addr ipDest;
   unsigned char reserved;
   unsigned char protocol;
   uint16_t tcpLength;

}__attribute__((packed));

struct udp {

   uint16_t src;
   uint16_t dest;
   uint16_t length;
   uint16_t checksum;

}__attribute__((packed));
 
