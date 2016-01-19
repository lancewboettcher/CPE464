/*
 * CPE 464
 * Program 1 - Trace
 * Lance Boettcher
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap/pcap.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "trace.h" 
#include "checksum.h"

#define ETHERNET_LENGTH 14
#define IP_LENGTH 20
#define IP_VERSION_LOC 4
#define DSCP_LOC 5
#define ICMP_CODE 1
#define TCP_CODE 6
#define UDP_CODE 17
#define TCP_OFFSET_LOC 2
#define ACK_FLAG_LOC 4

#define ICMP_REQUEST 8
#define ICMP_REPLY 0

#define ARP_REQUEST 256
#define ARP_REPLY 512

#define DNS 53
#define HTTP 80
#define Telnet 23
#define FTP 21
#define POP3 110
#define SMTP 25

void sniffTraceFile(char *filename);
void sniffIP(const u_char *loc);
void sniffARP(const u_char *loc);
void sniffUnknown(const u_char *loc);
void sniffProtocol(u_char type, const u_char *loc);
void sniffICMP(struct icmp *icmpHeader);
void sniffTCP(struct tcp *tcpHeader);
void sniffUDP(struct udp *udpHeader);
const char *getCommonPorts(uint16_t portNumber);
const char *yesOrNo(u_char val);

int main(int argc, char *argv[]) {

   if (argc != 2) {
      fprintf(stderr, "You must provide a trace file\n");
      exit(-1);
   }  
   else {
      sniffTraceFile(argv[1]);
   }

   return 0;
}

void sniffTraceFile(char *filename) {
   pcap_t *handle;
   char errorString[PCAP_ERRBUF_SIZE];
   struct pcap_pkthdr *header;
   const u_char *packet;
   struct ethernet *ethernetHeader;
   int packetNumber = 1;

   int nextOut;

   handle = pcap_open_offline(filename, errorString);

   if (handle == NULL) {
      fprintf(stderr, "Error opening tracefile: %s", filename);
      exit(-1);
   }

   while (nextOut = pcap_next_ex(handle, &header, &packet) == 1) {

      printf("\nPacket number: %d  Packet Len: %d\n\n", 
            packetNumber++, header->len);

      /* Ethernet Packet is always first */ 
      ethernetHeader = (struct ethernet *) packet;

      printf("\tEthernet Header\n");
      printf("\t\tDest MAC: %s\n", 
            ether_ntoa((const struct ether_addr *)ethernetHeader->dest)); 
      printf("\t\tSource MAC: %s\n", 
            ether_ntoa((const struct ether_addr *)ethernetHeader->src));

      if (ethernetHeader->type == TYPE_IP) {
         printf("\t\tType: IP\n\n");
         sniffIP(packet + ETHERNET_LENGTH);
      }
      else if (ethernetHeader->type == TYPE_ARP) {
         printf("\t\tType: ARP\n\n");
         sniffARP(packet + ETHERNET_LENGTH);
      }
      else {
         printf("\t\tType: Unknown\n\n");
         sniffUnknown(packet + ETHERNET_LENGTH);
      }
   }   

   pcap_close(handle);
}  

void sniffIP(const u_char *loc) {

   printf("\tIP Header\n");

   struct ip *ipHeader = (struct ip *) loc;
   int headerLength; 

   /* 
    * Version and Length
    * 0 | IP Version | 4 | IP Header Length | 7 
    */ 
   headerLength = (ipHeader->versionAndLength << 2) & 0x1F;
   printf("\t\tIP Version: %u\n", 
         ipHeader->versionAndLength >> IP_VERSION_LOC);
   printf("\t\tHeader Len (bytes): %u\n", headerLength);

   /* 
    * TOS
    * 0 | DSCP | 5 | ECN | 7 
    */ 
   printf("\t\tTOS subfields:\n");
   printf("\t\t   Diffserv bits: %u\n", ipHeader->tos >> DSCP_LOC);
   printf("\t\t   ECN bits: %u\n", ipHeader->tos & 0x03);
   
   /* TTL */ 
   printf("\t\tTTL: %u\n", ipHeader->ttl);

   /* Protocol */ 
   if (ipHeader->protocol == ICMP_CODE) 
      printf("\t\tProtocol: ICMP\n");
   else if (ipHeader->protocol == TCP_CODE) 
      printf("\t\tProtocol: TCP\n");
   else if (ipHeader->protocol == UDP_CODE)
      printf("\t\tProtocol: UDP\n");
   else 
      printf("\t\tProtocol: Unknown\n");

   /* Checksum */ 
   if (in_cksum((unsigned short *) ipHeader, headerLength * 4) == 0)
      printf("\t\tChecksum: Correct (0x%hx)\n", ntohs(ipHeader->checksum));
   else
      printf("\t\tChecksum: Incorrect (0x%hx)\n", ntohs(ipHeader->checksum));

   /* Sender IP */ 
   printf("\t\tSender IP: %s\n", inet_ntoa(ipHeader->src));

   /* Dest IP */ 
   printf("\t\tDest IP: %s\n\n", inet_ntoa(ipHeader->dest));

   sniffProtocol(ipHeader->protocol, loc + IP_LENGTH);
}

void sniffARP(const u_char *loc) {

   printf("\tARP header\n");

   struct arp *arpHeader = (struct arp *) loc;

   /* Opcode */ 
   if (arpHeader->opcode == ARP_REQUEST)
      printf("\t\tOpcode: Request\n");
   else if (arpHeader->opcode == ARP_REPLY) 
      printf("\t\tOpcode: Reply\n");  
   else 
      printf("\t\tOpcode: Unknown (%hu)\n", arpHeader->opcode);
   
   /* Sender MAC */ 
   printf("\t\tSender MAC: %s\n",
         ether_ntoa(&(arpHeader->senderMAC)));
   
   /* Sender IP */ 
   printf("\t\tSender IP: %s\n", inet_ntoa(arpHeader->senderIP));

   /* Target MAC */ 
   printf("\t\tTarget MAC: %s\n",
         ether_ntoa(&(arpHeader->targetMAC)));
   
   /* Target IP */ 
   printf("\t\tTarget IP: %s\n\n", inet_ntoa(arpHeader->targetIP));
}

void sniffUnknown(const u_char *loc) {


}

void sniffProtocol(u_char type, const u_char *loc) {

   switch (type) {
      case ICMP_CODE: 
         printf("\tICMP Header\n");

         struct icmp *icmpHeader = (struct icmp *) loc;

         sniffICMP(icmpHeader);

         break;
      case TCP_CODE: 
         printf("\tTCP Header\n");

         struct tcp *tcpHeader = (struct tcp *) (loc + 0);
   
         sniffTCP(tcpHeader);

         break;
      case UDP_CODE: 
         printf("UDP Header\n");

         struct udp *udpHeader = (struct udp *) loc;

         sniffUDP(udpHeader);

         break;
   }

}

void sniffICMP(struct icmp *icmpHeader) {

   /* Type */ 
   if (icmpHeader->type == ICMP_REQUEST)
      printf("\t\tType: Request\n");
   else if (icmpHeader->type == ICMP_REPLY)
      printf("\t\tType: Reply\n");
   else 
      printf("\t\tType: Other-Unknown\n");

}

void sniffTCP(struct tcp *tcpHeader) {

   printf("\t\tSource Port: %hu\n", ntohs(tcpHeader->src));
   printf("\t\tDest Port: %hu\n", ntohs(tcpHeader->dest)); 
   printf("\t\tSequence Number: %u\n", ntohl(tcpHeader->sequenceNumber));
   printf("\t\tACK Number: %u\n", ntohl(tcpHeader->ackNumber));
   printf("\t\tData Offset (bytes): %u\n", 
         (tcpHeader->offsetAndReserved >> TCP_OFFSET_LOC) & 0x1F); 

   printf("\t\tSYN Flag: %s\n", 
         yesOrNo((tcpHeader->flags >> 1) & 0x01));

   printf("\t\tRST Flag: %s\n", 
         yesOrNo((tcpHeader->flags >> 2) & 0x01));

   printf("\t\tFIN Flag: %s\n", 
         yesOrNo(tcpHeader->flags & 0x01));

   printf("\t\tACK Flag: %s\n", 
         yesOrNo((tcpHeader->flags >> ACK_FLAG_LOC) & 0x01));

   printf("\t\tWindow Size: %hu\n", ntohs(tcpHeader->windowSize));
  
   /* Checksum */  
}   

void sniffUDP(struct udp *udpHeader) {

   const char *src = getCommonPorts(ntohs(udpHeader->src));
   const char *dest = getCommonPorts(ntohs(udpHeader->dest));

   if (strlen(src) == 0)    
      printf("\t\tSource Port: %hu\n", ntohs(udpHeader->src));
   else 
      printf("\t\tSource Port: %s\n", src);
   if (strlen(dest) == 0)
      printf("\t\tDest Port: %hu\n\n", ntohs(udpHeader->dest));
   else 
      printf("\t\tDest Port: %s\n", dest);

}

const char *getCommonPorts(uint16_t portNumber) {
   switch (portNumber) {
      case DNS:
         return "DNS";
            
         break;
      case HTTP:
         return "HTTP";

         break;
      case Telnet:
         return "Telnet";
         
         break;
      case FTP: 
         return "FTP";
         
         break;
      case POP3: 
         return "POP3";
         
         break;
      case SMTP: 
         return "SMTP";

         break;
      default:

         return "";
   }
}   

const char *yesOrNo(u_char val) {
   if (val == 0x01)
      return "Yes";
   else 
      return "No";
}
