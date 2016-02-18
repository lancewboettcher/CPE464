/******************************************************************************
 * rcopy.c
 *  
 * CPE 464 - Program 3
 * Selective Reject
 *     
 * Lance Boettcher
 * ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "tcp_server.h"
#include "packets.h"
#include "testing.h"

struct rcopy rcopy;

int main(int argc, char *argv[]) {
   validateParams(argc, argv);

   initRCopy(argc, argv);

   runServer();

   return 0;
}

void validateParams(int argc, char *argv[]) {

   if (argc != 8) {
      printf("Usage: %s local-file remote-file buffer-size error-percent 
            window-size remote-machine remote-port\n", argv[0]);
      exit(-1);
   }

   if (strlen(argv[1]) > 1000) {
      printf("Local file name needs to be less than 1000 long\n");
      exit(-1);
   }
   if (strlen(argv[2]) > 1000) {
      printf("Remote file name needs to be less than 1000 long\n");
      exit(-1);
   }
   if (atoi(argv[3]) < 400 || atoi(argv[3]) > 1400) {
      printf("Buffer size must be between 400 and 1400\n");
      exit(-1);
   }
   if (atoi(argv[4]) < 0 || atoi(argv[4]) >= 1) {
      printf("Error percent must be between 0 and 1\n");
      exit(-1);
   }
   if (atoi(argv[5]) < 1) {
      printf("Window size must be greater than 0\n");
      exit(-1);
   }
}

void initRCopy(int argc, char *argv[]) {
   /* Initialize sendtoErr */
   sendtoErr_init(atof(argv[4]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);

   /* Setup  */
   rcopy.clientSocket = udp_send_setup(argv[6], atoi(argv[7]));

   /* Init sequence number */
   server.sequence = 1;
}

void sendFile() {




}

int tcp_send_setup(char *host_name, uint16_t *port) {
   int socket_num;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host
   
   if ((socket_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      perror("socket call");
      exit(-1);
   }

   //designate the addressing family
   remote.sin_family= AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL)
   {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
   }

   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port = htons(atoi(port));

   return socket_num;
}
