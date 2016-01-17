/* 
 * CPE 464
 * Program 1 - Trace
 * Lance Boettcher
 */

#define TYPE_IP 0x0800
#define TYPE_ARP 0x0806
#define 

struct ethernet {

   u_char dest[ETHERNET_LENGTH];
   u_char src[ETHERNET_LENGTH];
   u_short type;

}__attribute__((packed));

struct ip {


}__attribute__((packed));

struct tcp {


}__attribute__((packed));

struct udp {


}__attribute__((packed));

struct arp {


}__attribute__((packed));
