/*
 * CPE 464
 * Program 1 - Trace
 * Lance Boettcher
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <arpa/inet.h>
#include "trace.h" 
#include "checksum.h"

#define ETHERNET_LENGTH 14
#define IP_LENGTH 20
#define IP_VERSION_LOC 4
#define DSCP_LOC 5
#define ICMP_CODE 1
#define TCP_CODE 6
#define UDP_CODE 17

int main(int argc, char *argv[]) {

   if (argc != 2) {
      fprintf(stderr, "You must provide a trace file");
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
      fprintf(stderr, "Error opening tracefile: %s" argv[1]);
      exit(-1);
   }

   while (pcap_next_ex(handle, &header, &packet)) {
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

void sniffIP(u_char *loc) {

   printf("\tIP Header\n");

   ip ipHeader = (struct ip *) loc;

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
   if (protocol == ICMP_CODE) 
      printf("\t\tProtocol: ICMP\n");
   else if (protocol == TCP_CODE) 
      printf("\t\tProtocol: TCP\n");
   else if (protocol == UDP_CODE)
      printf("\t\tProtocol: UDP\n");
   else 
      printf("\t\tProtocol: Unknown\n");

   /* Checksum */ 


   /* Sender IP */ 
   printf("\t\tSender IP: %s\n", inet_ntoa(ipHeader->src));

   /* Dest IP */ 
   printf("\t\tDest IP: %s\n", inet_ntoa(ipHeader->dest));

   sniffProtocol(protocol, loc + IP_LENGTH);
}

void sniffARP(u_char *loc) {

   ip ipHeader = (struct ip *) 

}

void sniffUnknown(u_char *loc) {


}

void sniffProtocol(u_char type, u_char *loc) {

   switch (type) {
      case ICMP_CODE: 



         break;
      case TCP_CODE: 


         break;
      case UDP_CODE: 


         break;
      default: 
   }

}
