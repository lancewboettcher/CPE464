/* 
 * CPE 464
 * Program 1 - Trace
 * Lance Boettcher
 */

#define TYPE_IP 8
#define TYPE_ARP 1544

struct ethernet {

   u_char dest[6];
   u_char src[6];
   uint16_t type;

}__attribute__((packed));

struct ip {

   u_char versionAndLength;
   u_char tos;
   uint16_t size;
   uint16_t id;
   uint16_t fragOffset;
   u_char ttl;
   u_char protocol;
   uint16_t checksum;
   struct in_addr src;
   struct in_addr dest;

}__attribute__((packed));

struct arp {

   uint16_t hardwareType;
   uint16_t protocolType;
   u_char hardwareAddressLength;
   u_char protocolAddressLength;
   uint16_t opcode;
   struct ether_addr senderMAC;
   struct in_addr senderIP;
   struct ether_addr targetMAC;
   struct in_addr targetIP;

}__attribute__((packed));

struct icmp {

   u_char type;
   u_char code;
   uint16_t checksum;
   uint32_t restOfHeader;

}__attribute__((packed));

struct tcp {

   uint16_t src;
   uint16_t dest;
   uint32_t sequenceNumber;
   uint32_t ackNumber;
   u_char offsetAndReserved;
   u_char flags;
   uint16_t windowSize;
   uint16_t checksum; 
   uint16_t urgent; 
   uint32_t padding;

}__attribute__((packed));

struct tcpPseudo {

   struct in_addr ipSrc;
   struct in_addr ipDest;
   u_char reserved;
   u_char protocol;
   uint16_t tcpLength;

   uint16_t src;
   uint16_t dest;
   uint32_t sequenceNumber;
   uint32_t ackNumber;
   u_char offsetAndReserved;
   u_char flags;
   uint16_t windowSize;
   uint16_t checksum; 
   uint16_t urgent; 
   uint32_t padding;

}__attribute__((packed));

struct udp {

   uint16_t src;
   uint16_t dest;
   uint16_t length;
   uint16_t checksum;

}__attribute__((packed));
 
