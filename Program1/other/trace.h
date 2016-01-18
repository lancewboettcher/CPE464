#ifndef _H_TRACE
#define _H_TRACE

#define ETHER_TYPE_IP  0x0800
#define ETHER_TYPE_ARP 0x0806

#define ARP_OPCODE_REQUEST 0x0001
#define ARP_OPCODE_REPLY   0x0002

#define IP_PROTOCOL_ICMP 0x01
#define IP_PROTOCOL_TCP  0x06
#define IP_PROTOCOL_UDP  0x11

#define ICMP_TYPE_REPLY   0x00
#define ICMP_TYPE_REQUEST 0x08

#define MAX_PACKET_SIZE 65535

struct ether_hdr 
{
       struct ether_addr dst;
           struct ether_addr src;
               uint16_t type;
} __attribute__((packed));

struct arp_hdr
{
       uint16_t htype;
           uint16_t ptype;
               u_char hlen;
                   u_char plen;
                       uint16_t oper;
                           struct ether_addr sha;
                               struct in_addr spa;
                                   struct ether_addr tha;
                                       struct in_addr tpa;
} __attribute__((packed));

struct ip_hdr
{
       u_char version_ihl;
           u_char tos_ecn;
               uint16_t length;
                   uint16_t identification;
                       uint16_t flags_offset;
                           u_char ttl;
                               u_char protocol;
                                   uint16_t checksum;
                                       struct in_addr src;
                                           struct in_addr dst;
} __attribute__((packed));

struct icmp_hdr 
{
       u_char type;
           u_char code;
               uint16_t checksum;
                   uint32_t rest;
} __attribute__((packed));

struct tcp_hdr
{
       uint16_t src_port;
           uint16_t dst_port;
               uint32_t seq_num;
                   uint32_t ack_num;
                       u_char off_res;
                           u_char flags;
                               uint16_t window_size;
                                   uint16_t checksum;
} __attribute__((packed));

struct tcp_pshdr 
{
       struct in_addr src_ip;
           struct in_addr dst_ip;
               u_char zeros;
                   u_char protocol;
                       uint16_t tcp_len;
} __attribute__((packed));

struct udp_hdr
{
       uint16_t src_port;
           uint16_t dst_port;
               uint16_t length;
                   uint16_t checksum;
} __attribute__((packed));

void process_packet(u_char *args, const struct pcap_pkthdr *header, 
          const u_char *packet);
void analyze_arp(const u_char *packet);
void analyze_ip(const u_char *packet);
void analyze_icmp(const u_char *packet, int ihl);
void analyze_tcp(const u_char *packet, int ihl);
void analyze_udp(const u_char *packet, int ihl);

#endif
