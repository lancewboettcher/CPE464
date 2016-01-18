/* 
 * CPE 464
 * Program 1 - Trace
 * Lance Boettcher
 */

#define TYPE_IP 0x0800
#define TYPE_ARP 0x0806

struct ethernet {

   struct ether_addr dest;
   struct ether_addr src;
   u_short type;

}__attribute__((packed));

struct ip {

   u_char versionAndLength;
   u_char tos;
   u_short size;
   u_short id;
   u_short fragOffset;
   u_char ttl;
   u_char protocol;
   u_short checksum;
   struct in_addr src;
   struct in_addr dest;

}__attribute__((packed));

struct tcp {

   u_short sourcePort;
   u_short destPort;
   uint32_t sequenceNum;
   uint32_t ackNum;
   u_char dataOffset;
   u_char flags;
   u_short window;
   u_short checksum;
   u_short urgent;

}__attribute__((packed));

struct udp {

   uint16_t src;
   uint16_t dest;
   uint16_t length;
   uint16_t checksum;

}__attribute__((packed));

struct arp {

   uint16_t hardwareType;
   uint16_t protocolType;
   u_char hardwareAddressLength;
   u_char protocolAddressLength;
   uint16_t opcode;
   struct ether_addr senderHardwareAddress;
   struct in_addr senderProtocolAddress;
   struct ether_addr targetHardwareAddress;
   struct in_addr targetProtocolAddress;

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
   uint16_t urgent; //Maybe not?
   uint32_t padding;

}__attribute__((packed));

struct udp {

   uint16_t src;
   uint16_t dest;
   uint16_t length;
   uint16_t checksum;

}__attribute__((packed));
 
