/******************************************************************************
 * server.c
 *
 * CPE 464 - Program 3
 * Selective Reject
 *
 * Lance Boettcher
 ******************************************************************************/

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

struct server server;

int main(int argc, char *argv[]) {
   initServer(argc, argv);

   runServer();

   return 0;
}

void initServer(int argc, char *argv[]) {
   /* Initialize sendtoErr */ 
   if (argc >= 2) // Error Percent
      sendtoErr_init(atof(argv[1]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON); 
   else {
      perror("Usage: server <Error Percent> <Port Number (Optional)>");
      exit(-1);
   }

   /* Create server Socket */
   if (argc > 2) // Port Number Specified
      server.serverSocket = udp_recv_setup(atoi(argv[2]));
   else 
      server.serverSocket = udp_recv_setup(0);

   /* Init sequence number */ 
   server.sequence = 1;
}

void runServer() {




}

int udp_recv_setup(int portNumber) {

   int server_socket= 0;
   struct sockaddr_in local;      /* socket address for local side  */
   socklen_t len = sizeof(local);  /* length of local address        */

   /* create the socket  */
   server_socket= socket(AF_INET, SOCK_DGRAM, 0);
   if(server_socket < 0) {
      perror("socket call");
      exit(1);
   }

   local.sin_family= AF_INET;         //internet family
   local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
   local.sin_port= htons(portNumber);

   /* bind the name (address) to a port */
   if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
      perror("bind call");
      exit(-1);
   }

   //get the port name and print it out
   if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0) {
      perror("getsockname call");
      exit(-1);
   }

   printf("Server is using port %d\n", ntohs(local.sin_port));

   return server_socket;
}

