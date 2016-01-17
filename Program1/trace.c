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

int main(int argc, char *argv[]) {

   if (argc != 2) {
      fprintf(stderr, "You must provide a trace file");
      exit(-1);
   }

   sniffTraceFile(argv[1]);
}

void sniffTraceFile(char *filename) {
   pcap_t *handle;
   char errorString[PCAP_ERRBUF_SIZE];
   struct pcap_pkthdr *header;
   const u_char *packet;
   struct ethernet *ethernet;

   handle = pcap_open_offline(filename, errorString);

   if (handle == NULL) {
      fprintf(stderr, "Error opening tracefile: %s" argv[1]);
      exit(-1);
   }

   while (pcap_next_ex(handle, &header, &packet)) {
      ethernet = (struct ethernet*) packet;

      if (ethernet->type == TYPE_IP) {
         sniffIP(header, packet);
      }
      else if (ethernet->type == TYPE_ARP) {
         sniffARP(header, packet);
      }
      else {
         sniffUnknown(header, packet);
      }
   }   

   pcap_close(handle);
}  

void sniffIP(u_char *header, u_char *packet) {



}

void sniffARP(u_char *header, u_char *packet) {


}

void sniffUnknown(u_char *header, u_char *packet) {


}
