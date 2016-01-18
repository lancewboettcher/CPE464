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
#define TCP_OFFSET_LOC 4
#define ACK_FLAG_LOC 4

#define ICMP_REQUEST 0
#define ICMP_REPLY 8

#define ARP_REQUEST 1
#define ARP_REPLY 2


void sniffTraceFile(char *filename);
void sniffIP(const u_char *loc);
void sniffARP(const u_char *loc);
void sniffUnknown(const u_char *loc);
void sniffProtocol(u_char type, const u_char *loc);
void sniffICMP(struct icmp *icmpHeader);
void sniffTCP(struct tcp *tcpHeader);
void sniffUDP(struct udp *udpHeader);
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

   handle = pcap_open_offline(filename, errorString);

   if (handle == NULL) {
      fprintf(stderr, "Error opening tracefile: %s", filename);
      exit(-1);
   }

   while (pcap_next_ex(handle, &header, &packet) == 1) {
      printf("Packet number: %d Packet Len: %d\n\n", 
            packetNumber++, header->len);

      /* Ethernet Packet is always first */ 
      ethernetHeader = (struct ethernet *) packet;

      printf("\tEthernet Header\n\t\tDest MAC: %s\n\t\tSource MAC: %s\n",
            ether_ntoa(&(ethernetHeader->dest)), 
            ether_ntoa(&(ethernetHeader->src)));

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

   /* 
    * Version and Length
    * 0 | IP Version | 4 | IP Header Length | 7 
    */ 
   printf("\t\tIP Version: %u\n", 
         ipHeader->versionAndLength >> IP_VERSION_LOC);
   printf("\t\tHeader Len (bytes): %u\n", 
         ipHeader->versionAndLength & 0x0F);

   /* 
    * TOS
    * 0 | DSCP | 5 | ECN | 7 
    */ 
   printf("\tTOS subfields:\n");
   printf("\t\tDiffserv bits: %u\n", ipHeader->tos >> DSCP_LOC);
   printf("\t\tECN bits: %u\n", ipHeader->tos & 0x03);
   
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


   /* Sender IP */ 
   printf("\t\tSender IP: %s\n", inet_ntoa(ipHeader->src));

   /* Dest IP */ 
   printf("\t\tDest IP: %s\n\n", inet_ntoa(ipHeader->dest));

   sniffProtocol(ipHeader->protocol, loc + IP_LENGTH);
}

void sniffARP(const u_char *loc) {

   printf("\tARP heder\n");

   struct arp *arpHeader = (struct arp *) loc;

   /* Opcode */ 
   if (arpHeader->opcode == ARP_REQUEST)
      printf("\t\tOpcode: Request\n");
   else if (arpHeader->opcode == ARP_REPLY) 
      printf("\t\tOpcode: Reply\n");   
   
   /* Sender MAC */ 
   printf("\t\tSender MAC: %s\n",
         ether_ntoa(&(arpHeader->senderMAC)));
   
   /* Sender IP */ 
   printf("\t\tSender IP: %s\n", inet_ntoa(arpHeader->senderIP));

   /* Target MAC */ 
   printf("\t\tTarget MAC: %s\n",
         ether_ntoa(&(arpHeader->targetMAC)));
   
   /* Target IP */ 
   printf("\t\tSender IP: %s\n\n", inet_ntoa(arpHeader->targetIP));
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

         struct tcp *tcpHeader = (struct tcp *) loc;

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
      printf("\t\tType: Request\n\n");
   else if (icmpHeader->type == ICMP_REPLY)
      printf("\t\tType: Reply\n\n");
   else 
      printf("\t\tType: Other-Unknown\n\n");

}

void sniffTCP(struct tcp *tcpHeader) {

   printf("\t\tSource Port: %u\n", tcpHeader->src);
   printf("\t\tDest Port: %u\n", tcpHeader->dest); 
   printf("\t\tSequence Number: %u\n", tcpHeader->sequenceNumber);
   printf("\t\tACK Number: %u\n", tcpHeader->ackNumber);
   printf("\t\tData Offset (bytes): %u\n", 
         tcpHeader->offsetAndReserved >> TCP_OFFSET_LOC); 

   printf("\t\tSYN FLAG: %s\n", 
         yesOrNo((tcpHeader->flags >> 1) & 0x01));

   printf("\t\tRST FLAG: %s\n", 
         yesOrNo((tcpHeader->flags >> 2) & 0x01));

   printf("\t\tFIN FLAG: %s\n", 
         yesOrNo(tcpHeader->flags & 0x01));

   printf("\t\tACK FLAG: %s\n", 
         yesOrNo((tcpHeader->flags >> ACK_FLAG_LOC) & 0x01));

   printf("\t\tWindow Size: %u\n", tcpHeader->windowSize);
  
   /* Checksum */  
}   

void sniffUDP(struct udp *udpHeader) {

   printf("\t\tSource Port: %u\n", udpHeader->src);
   printf("\t\tDest Port: %u\n", udpHeader->dest); 

}

const char *yesOrNo(u_char val) {
   if (val == 0x01)
      return "yes";
   else 
      return "no";
}
