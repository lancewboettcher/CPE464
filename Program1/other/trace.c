#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "trace.h"
#include "checksum.h"

int count = 0;

int main(int argc, char *argv[]) 
{
       pcap_t *handle;
           char errbuf[PCAP_ERRBUF_SIZE];

               /* Test for the correct number of arguments */
               if (argc != 2) {
                          fprintf(stderr, "usage: trace aTraceFile\n");
                                  return EXIT_FAILURE;
                                      }

                   /* Open the packet trace file */
                   handle = pcap_open_offline(argv[1], errbuf);
                       if (handle == NULL) {
                                  fprintf(stderr, "%s\n", errbuf);
                                          return EXIT_FAILURE;
                                              }
                           pcap_loop(handle, -1, process_packet, NULL);
                               pcap_close(handle);

                                   return EXIT_SUCCESS;
}

void process_packet(u_char *args, const struct pcap_pkthdr *header, 
          const u_char *packet)
{
       struct ether_hdr *hdr = (struct ether_hdr *)packet;

           printf("\nPacket number: %d  Packet Len: %d\n\n", ++count, 
                         header->len);

               printf("\tEthernet Header\n");
                   printf("\t\tDest MAC: %s\n", ether_ntoa(&hdr->dst));
                       printf("\t\tSource MAC: %s\n", ether_ntoa(&hdr->src));

                           switch (ntohs(hdr->type)) {
                                  case ETHER_TYPE_IP:
                                             printf("\t\tType: IP\n");
                                                     analyze_ip(packet);
                                                             break;
                                                                 case ETHER_TYPE_ARP:
                                                                     printf("\t\tType: ARP\n");
                                                                             analyze_arp(packet);
                                                                                     break;
                                                                                         default:
                                                                                             printf("\t\tType: Unknown\n");
                                                                                                 }
}

void analyze_arp(const u_char *packet)
{
       char *opcode;
           struct arp_hdr *hdr = (struct arp_hdr *)&packet[sizeof(struct ether_hdr)];

               switch (ntohs(hdr->oper)) {
                      case ARP_OPCODE_REQUEST:
                                 opcode = "Request";
                                         break;
                                             case ARP_OPCODE_REPLY:
                                                 opcode = "Reply";
                                                         break;
                                                             default:
                                                                 opcode = "Unknown";
                                                                     }

                   printf("\n\tARP header\n");
                       printf("\t\tOpcode: %s\n", opcode);
                           printf("\t\tSender MAC: %s\n", ether_ntoa(&hdr->sha));
                               printf("\t\tSender IP: %s\n", inet_ntoa(hdr->spa));
                                   printf("\t\tTarget MAC: %s\n", ether_ntoa(&hdr->tha));
                                       printf("\t\tTarget IP: %s\n\n", inet_ntoa(hdr->tpa));
}

void analyze_ip(const u_char *packet)
{
       char *protocol;
           struct ip_hdr *hdr = (struct ip_hdr *)&packet[sizeof(struct ether_hdr)];
               int ihl = hdr->version_ihl & 0x0F;
                   unsigned short checksum = 
                              in_cksum((unsigned short *)hdr, ihl * 4);

                       switch (hdr->protocol) {
                              case IP_PROTOCOL_ICMP:
                                         protocol = "ICMP";
                                                 break;
                                                     case IP_PROTOCOL_TCP:
                                                         protocol = "TCP";
                                                                 break;
                                                                     case IP_PROTOCOL_UDP:
                                                                         protocol = "UDP";
                                                                                 break;
                                                                                     default:
                                                                                         protocol = "Unknown";
                                                                                                 break;
                                                                                                     }

                           printf("\n\tIP Header\n");
                               printf("\t\tTOS: 0x%x\n", hdr->tos_ecn);
                                   printf("\t\tTTL: %d\n", hdr->ttl);
                                       printf("\t\tProtocol: %s\n", protocol);
                                           printf("\t\tChecksum: %s (0x%hx)\n", checksum ? "Incorrect" : "Correct", 
                                                             ntohs(hdr->checksum));
                                               printf("\t\tSender IP: %s\n", inet_ntoa(hdr->src));
                                                   printf("\t\tDest IP: %s\n",  inet_ntoa(hdr->dst));

                                                       switch (hdr->protocol) {
                                                              case IP_PROTOCOL_ICMP:
                                                                         analyze_icmp(packet, ihl);
                                                                                 break;
                                                                                     case IP_PROTOCOL_TCP:
                                                                                         analyze_tcp(packet, ihl);
                                                                                                 break;
                                                                                                     case IP_PROTOCOL_UDP:
                                                                                                         analyze_udp(packet, ihl);
                                                                                                                 break;
                                                                                                                     }
}

void analyze_icmp(const u_char *packet, int ihl)
{
       char *type;
           struct icmp_hdr *hdr = (struct icmp_hdr *)&packet[sizeof(struct ether_hdr) 
                      + ihl * 4];

               switch (hdr->type) {
                      case ICMP_TYPE_REPLY:
                                 type = "Reply";
                                         break;
                                             case ICMP_TYPE_REQUEST:
                                                 type = "Request";
                                                         break;
                                                             default:
                                                                 type = "Unknown";
                                                                         break;
                                                                             }

                   printf("\n\tICMP Header\n");
                       printf("\t\tType: %s\n", type);
}

char * port_service(uint16_t port) 
{
       switch (port) {
              case 20:
                         return "FTP";
                             case 21:
                                 return "FTP";
                                     case 23:
                                         return "Telnet";
                                             case 25:
                                                 return "SMTP";
                                                     case 80:
                                                         return "HTTP";
                                                             case 110:
                                                                 return "POP3";
                                                                     default:
                                                                         return NULL;
                                                                             }
}

void analyze_tcp(const u_char *packet, int ihl)
{
       char *service;
           unsigned short checksum;
               int syn_flag, rst_flag, fin_flag;
                   uint16_t total_len, tcp_segment_len;

                       struct ip_hdr *iphdr = (struct ip_hdr *)&packet[sizeof(struct ether_hdr)];
                           struct tcp_hdr *hdr = (struct tcp_hdr *)
                                      &packet[sizeof(struct ether_hdr) + ihl * 4];
                               struct tcp_pshdr pshdr;

                                   u_char *datagram = malloc(MAX_PACKET_SIZE);
                                       memset(&pshdr, 0, sizeof(struct tcp_pshdr));

                                           total_len = ntohs(iphdr->length);
                                               tcp_segment_len = total_len - (uint16_t)sizeof(struct ip_hdr);

                                                   /* Construct the pseudo header and copy into datagram */
                                                   pshdr.src_ip = iphdr->src; 
                                                       pshdr.dst_ip = iphdr->dst;
                                                           pshdr.protocol = iphdr->protocol;
                                                               pshdr.tcp_len = htons(tcp_segment_len);
                                                                   memcpy(datagram, &pshdr, sizeof(struct tcp_pshdr));

                                                                       /* Copy TCP header and data into datagram, then calculate checksum */
                                                                       memcpy(&datagram[sizeof(struct tcp_pshdr)], hdr, tcp_segment_len);
                                                                           
                                                                       
                                                                       printf("\n\nCHECKSUM\n");
                                                 printf("length: %d\n", sizeof(struct tcp_pshdr) + tcp_segment_len);
                                                 printf("psh Length %hu\n\n", tcp_segment_len);                      
                                                                       
                                                                       checksum = in_cksum((unsigned short *)datagram, sizeof(struct tcp_pshdr) + 
                                                                                             tcp_segment_len);

                                                                               /* Calculate flag values from flags field */
                                                                               syn_flag = (hdr->flags) & (1 << 1);
                                                                                   rst_flag = (hdr->flags) & (1 << 2);
                                                                                       fin_flag = (hdr->flags) & (1 << 0);

                                                                                           printf("\n\tTCP Header\n");

                                                                                               /* Translate port numbers into service names (if possible) */
                                                                                               service = port_service(ntohs(hdr->src_port));
                                                                                                   if (service)
                                                                                                              printf("\t\tSource Port:  %s\n", service);
                                                                                                       else
                                                                                                                  printf("\t\tSource Port:  %hu\n", ntohs(hdr->src_port));

                                                                                                           service = port_service(ntohs(hdr->dst_port));
                                                                                                               if (service)
                                                                                                                          printf("\t\tDest Port:  %s\n", service);
                                                                                                                   else
                                                                                                                              printf("\t\tDest Port:  %hu\n", ntohs(hdr->dst_port));

                                                                                                                       printf("\t\tSequence Number: %u\n", ntohl(hdr->seq_num));
                                                                                                                           printf("\t\tACK Number: %u\n", ntohl(hdr->ack_num));
                                                                                                                               
                                                                                                                               printf("\t\tSYN Flag: %s\n", syn_flag ? "Yes" : "No");
                                                                                                                                   printf("\t\tRST Flag: %s\n", rst_flag ? "Yes" : "No");
                                                                                                                                       printf("\t\tFIN Flag: %s\n", fin_flag ? "Yes" : "No");

                                                                                                                                           printf("\t\tWindow Size: %hu\n", ntohs(hdr->window_size));
                                                                                                                                               printf("\t\tChecksum: %s (0x%hx)\n", checksum ? "Incorrect" : "Correct", 
                                                                                                                                                                 ntohs(hdr->checksum));
}

void analyze_udp(const u_char *packet, int ihl)
{
       struct udp_hdr *hdr = (struct udp_hdr *)&packet[sizeof(struct ether_hdr) +
                  ihl * 4];
           printf("\n\tUDP Header\n");
               printf("\t\tSource Port:  %hu\n", ntohs(hdr->src_port));
                   printf("\t\tDest Port:  %hu\n", ntohs(hdr->dst_port));
}
