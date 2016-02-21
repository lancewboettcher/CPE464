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

#include "util.h"
#include "rcopy.h"
#include "cpe464.h"

struct rcopy rcopy;
Connection server;

int main(int argc, char *argv[]) {
   validateParams(argc, argv);

   initRCopy(argc, argv);

   sendFile(argc, argv);

   return 0;
}

void initRCopy(int argc, char *argv[]) {
   /* Initialize sendtoErr */
   sendtoErr_init(atof(argv[4]), DROP_OFF, FLIP_OFF, DEBUG_OFF, RSEED_OFF);

   /* Init sequence number */
   rcopy.sequence = 1;
}

void sendFile(int argc, char *argv[]) {

   int32_t select_count = 0;
   STATE state = FILENAME;

   while (state != DONE) {

      switch (state) {
         case FILENAME: 

            /* Setup new socket */
            if (udp_client_setup(argv[6], atoi(argv[7])) < 0) {
               printf("Host not found\n");
               exit(-1);
            }

            state = filename(argv[1], atoi(argv[3]));

            if (state == FILENAME)
               close(server.sk_num);

            select_count++;
            if (select_count > 9) {
               printf("Server unreachable, client terminating\n");
               state = DONE;
            }

            break;
         case FILE_OK: 


            break;
         case RECV: 


            break;
         default: 
            printf("Error - in default state\n");
            break;
      }

   }
}

STATE filename(char *fname, int32_t buf_size) {

   uint8_t packet[MAX_LEN];
   uint8_t buf[MAX_LEN];
   uint8_t flag = 0;
   int32_t seq_num = 0;
   int32_t fname_len = strlen(fname) + 1;
   int32_t recv_check = 0;

   memcpy(buf, &buf_size, 4);
   memcpy(&buf[4], fname, fname_len);

   printf("Port: %hu socket: %d\n", ntohs(server.remote.sin_port), server.sk_num);

   send_buf(buf, fname_len + 4, &server, FNAME, 0, packet);

   if (select_call(server.sk_num, 1, 0, NOT_NULL) == 1) {

      recv_check = recv_buf(packet, 1000, server.sk_num, &server, &flag, &seq_num);

      if (recv_check == CRC_ERROR) {
         printf("CRC_ERROR in filename\n");
         return FILENAME;
      }
      if (flag == FNAME_BAD) {
         printf("File %s not found\n", fname);
         return DONE;
      }

      return FILE_OK;
   }

   return FILENAME;
}
/*
int udp_send_setup(char *host_name, int port) {
   int socket_num;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp = NULL;              // address of remote host

   if ((socket_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket call");
      exit(-1);
   }

   //designate the addressing family
   remote.sin_family= AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL) {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
   }

   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port = htons(port);

   return socket_num;
}
*/

int udp_client_setup(char *hostname, uint16_t port_num) {
   /* returns a pointer to a sockaddr_in that it created or NULL if host not found.
    *     * also passes back the socket number in sk */

   struct hostent *hp = NULL; // address of remote host

   server.sk_num = 0;
   server.len = sizeof(struct sockaddr_in);

   if ((server.sk_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("udp_client_setup, socket");
      exit(-1);
   }

   printf("Socket num: %d\n", server.sk_num);

   server.remote.sin_family = AF_INET;

   hp = gethostbyname(hostname);

   if (hp == NULL) {
      printf("Host not found: %s\n", hostname);
      return -1;
   }

   printf("Client setup. addr: %s, length: %d\n", hp->h_name, hp->h_length);

   memcpy(&(server.remote.sin_addr), hp->h_addr, hp->h_length);

   server.remote.sin_port = htons(port_num);

   return 0;
}

void validateParams(int argc, char *argv[]) {

   if (argc != 8) {
      printf("Usage: %s local-file remote-file buffer-size error-percent"
            "window-size remote-machine remote-port\n", argv[0]);
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
