/*
 * CPE 464
 * Program 1 - Trace
 * Lance Boettcher
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap/pcap.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include "trace.h" 
#include "checksum.h"

void sniffEthernet(char *filename);
void sniffIP(const unsigned char *loc);
void sniffARP(const unsigned char *loc);
void sniffProtocol(unsigned char type, const unsigned char *loc, 
      struct tcpPseudo pseudo, int ip);
void sniffICMP(struct icmp *icmpHeader, int ip);
void sniffTCP(struct tcp *tcpHeader, struct tcpPseudo pseudo);
void tcpChecksum(struct tcpPseudo pseudo, struct tcp *tcpHeader);
void sniffUDP(struct udp *udpHeader);
const char *getCommonPorts(uint16_t portNumber);
int ipVersionAndLength(struct ip *ipHeader);
void ipTOS(struct ip *ipHeader);
const char *yesOrNo(unsigned char val);

int main(int argc, char *argv[]) {

   if (argc != 2) {
      fprintf(stderr, "You must provide a trace file\n");
      exit(-1);
   }  
   else {
      sniffEthernet(argv[1]);
   }

   return 0;
}

void sniffEthernet(char *filename) {
   pcap_t *handle;
   char errorString[PCAP_ERRBUF_SIZE];
   struct pcap_pkthdr *header;
   const unsigned char *packet;
   struct ethernet *ethernetHeader;
   int packetNumber = 1;

   handle = pcap_open_offline(filename, errorString);

   if (handle == NULL) {
      fprintf(stderr, "Error opening tracefile: %s", filename);
      exit(-1);
   }

   while (pcap_next_ex(handle, &header, &packet) == 1) {

      printf("\nPacket number: %d  Packet Len: %d\n\n", 
            packetNumber++, header->len);

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
      }
   }   

   pcap_close(handle);
}  

void sniffIP(const unsigned char *loc) {

   printf("\tIP Header\n");

   struct ip *ipHeader = (struct ip *) loc;

   int ipV = ipVersionAndLength(ipHeader); 

   ipTOS(ipHeader);
   
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
   if (in_cksum((unsigned short *) ipHeader, IP_LENGTH) == 0)
      printf("\t\tChecksum: Correct (0x%04hx)\n", ntohs(ipHeader->checksum));
   else
      printf("\t\tChecksum: Incorrect (0x%04hx)\n", ntohs(ipHeader->checksum));

   /* Sender IP */ 
   printf("\t\tSender IP: %s\n", inet_ntoa(ipHeader->src));

   /* Dest IP */ 
   printf("\t\tDest IP: %s\n", inet_ntoa(ipHeader->dest));

   /* Set IP info for TCP Pseudo Header */ 
   struct tcpPseudo pseudo;

   pseudo.ipSrc = ipHeader->src;
   pseudo.ipDest = ipHeader->dest;
   pseudo.reserved = 0;
   pseudo.protocol = ipHeader->protocol; 
   pseudo.tcpLength = htons(ntohs(ipHeader->size) - IP_LENGTH);

   sniffProtocol(ipHeader->protocol, loc + IP_LENGTH, pseudo, ipV);
}

void sniffARP(const unsigned char *loc) {

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

void sniffProtocol(unsigned char type, const unsigned char *loc,
      struct tcpPseudo pseudo, int ip) {

   switch (type) {
      case ICMP_CODE: 
         printf("\n\tICMP Header\n");

         struct icmp *icmpHeader = (struct icmp *) loc;

         sniffICMP(icmpHeader, ip);

         break;
      case TCP_CODE: 
         printf("\n\tTCP Header\n");

         struct tcp *tcpHeader = (struct tcp *) loc;
   
         sniffTCP(tcpHeader, pseudo);

         break;
      case UDP_CODE: 
         printf("\n\tUDP Header\n");

         struct udp *udpHeader = (struct udp *) loc;

         sniffUDP(udpHeader);

         break;
   }

}

void sniffICMP(struct icmp *icmpHeader, int ip) {

   if (ip == BAD_IP) 
      printf("\t\tType: %d\n", BAD_IP_TYPE);
   else if (icmpHeader->type == ICMP_REQUEST)
      printf("\t\tType: Request\n");
   else if (icmpHeader->type == ICMP_REPLY)
      printf("\t\tType: Reply\n");
   else 
      printf("\t\tType: %u\n", icmpHeader->type);

}

void sniffTCP(struct tcp *tcpHeader, struct tcpPseudo pseudo) {
   /* Ports */ 
   const char *src = getCommonPorts(ntohs(tcpHeader->src));
   const char *dest = getCommonPorts(ntohs(tcpHeader->dest));

   if (strlen(src) == 0)    
      printf("\t\tSource Port:  %hu\n", ntohs(tcpHeader->src));
   else 
      printf("\t\tSource Port:  %s\n", src);
   if (strlen(dest) == 0)
      printf("\t\tDest Port:  %hu\n", ntohs(tcpHeader->dest));
   else 
      printf("\t\tDest Port:  %s\n", dest);
   
   printf("\t\tSequence Number: %u\n", ntohl(tcpHeader->sequenceNumber));
   
   printf("\t\tACK Number: %u\n", ntohl(tcpHeader->ackNumber));
   
   printf("\t\tData Offset (bytes): %u\n", 
         ((tcpHeader->offsetAndReserved >> TCP_OFFSET_LOC) & 0x0F)
         * sizeof(int)); 

   printf("\t\tSYN Flag: %s\n", 
         yesOrNo((tcpHeader->flags >> 1) & 0x01));

   printf("\t\tRST Flag: %s\n", 
         yesOrNo((tcpHeader->flags >> 2) & 0x01));

   printf("\t\tFIN Flag: %s\n", 
         yesOrNo(tcpHeader->flags & 0x01));

   printf("\t\tACK Flag: %s\n", 
         yesOrNo((tcpHeader->flags >> ACK_FLAG_LOC) & 0x01));

   printf("\t\tWindow Size: %hu\n", ntohs(tcpHeader->windowSize));
  
   tcpChecksum(pseudo, tcpHeader);
}  

void sniffUDP(struct udp *udpHeader) {

   const char *src = getCommonPorts(ntohs(udpHeader->src));
   const char *dest = getCommonPorts(ntohs(udpHeader->dest));

   if (strlen(src) == 0)    
      printf("\t\tSource Port:  %hu\n", ntohs(udpHeader->src));
   else 
      printf("\t\tSource Port:  %s\n", src);
   if (strlen(dest) == 0)
      printf("\t\tDest Port:  %hu\n", ntohs(udpHeader->dest));
   else 
      printf("\t\tDest Port:  %s\n", dest);

}

/* Prints the IP Version and Header Length. 
 * Returns IP Version */ 
int ipVersionAndLength(struct ip *ipHeader) {
   /* 
    * Version and Length
    * 0 | IP Version | 4 | IP Header Length | 7 
    */ 
   int headerLength = (ipHeader->versionAndLength << 2) & 0x3F;
   int ipV = ipHeader->versionAndLength >> IP_VERSION_LOC;
   printf("\t\tIP Version: %u\n", ipV);
   printf("\t\tHeader Len (bytes): %u\n", headerLength);
   
   return ipV;
}

/* Prints the TOS subfields of the IP Packet */ 
void ipTOS(struct ip *ipHeader) {

   /* 
    * TOS
    * 0 | DSCP | 5 | ECN | 7 
    */ 
   printf("\t\tTOS subfields:\n");
   printf("\t\t   Diffserv bits: %u\n", (ipHeader->tos >> DSCP_LOC) & 0x3F);
   printf("\t\t   ECN bits: %u\n", ipHeader->tos & 0x03);

}

/* Prints the correct checksum output for TCP Packet */ 
void tcpChecksum(struct tcpPseudo pseudo, struct tcp *tcpHeader) {

   /* Allocate a block for our checksum data to be send to in_cksum */   
   char *checksum = calloc(TCP_CHECKSUM_SIZE, sizeof(int));

   /* Copy the pseudo header to our checksum data */ 
   memcpy(checksum, &pseudo, TCP_PSEUDO_LENGTH);

   /* Copy the TCP Header AND DATA to our checksum data */ 
   memcpy(checksum + TCP_PSEUDO_LENGTH, tcpHeader, ntohs(pseudo.tcpLength));

   if (in_cksum((unsigned short *)checksum, 
            ntohs(pseudo.tcpLength) + TCP_PSEUDO_LENGTH) == 0)
      printf("\t\tChecksum: Correct (0x%04hx)\n", ntohs(tcpHeader->checksum));
   else
      printf("\t\tChecksum: Incorrect (0x%04hx)\n", ntohs(tcpHeader->checksum));

}

/* Returns a string of a common port number or empty string */ 
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
         /* Return empty string for non common ports */ 
         return "";
   }
}   

/* Returns yes or no strings given unsigned char 0, 1 */ 
const char *yesOrNo(unsigned char val) {
   if (val == 0x01)
      return "Yes";
   else 
      return "No";
}
