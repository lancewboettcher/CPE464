/******************************************************************************
 * tcp_client.c
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "testing.h"
#include "tcp_client.h"
#include "packets.h"

struct tcpClient tcpClient;

int main(int argc, char * argv[])
{
   /* check command line arguments  */
   if (validateParams(argc, argv) != 0) {
      printf("usage: %s handle host-name port-number \n", argv[0]);
      exit(-1);
   }

   initClient(argv);
/*
   int socket_num;         //socket descriptor
   char *send_buf;         //data buffer
   int bufsize= 0;         //data buffer size
   int send_len= 0;        //amount of data to send
   int sent= 0;            //actual amount of data sent

   // set up the socket for TCP transmission  
   socket_num= tcp_send_setup(argv[1], argv[2]);

   // initialize data buffer for the packet 
   bufsize= 1024;
   send_buf= (char *) malloc(bufsize);

   // get the data and send it   
   printf("Enter the data to send: ");

   send_len = 0;
   while ((send_buf[send_len] = getchar()) != '\n' && send_len < 80)
      send_len++;

   send_buf[send_len] = '\0';

   // now send the packet 
   sent =  send(socket_num, send_buf, send_len, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }
*/

   
   return 0;
}

int validateParams(int argc, char *argv[]) {
   if (argc != 4) 
      return -1;
   
   if (strlen(argv[1]) <= 0 || strlen(argv[2]) <= 0 || strlen(argv[3]) <= 0)
      return -1;

   return 0;
}

void initClient(char *argv[]) {
   struct initCtoS initPacket;
   char *packet;
   int sent, responseLength;
   char *buffer[BUFFER_SIZE];

   initPacket.handleLength = strlen(argv[1]);
   strcpy(tcpClient.handle, argv[1]);
   
   tcpClient.socketNum = tcp_send_setup(argv[2], argv[3]);

   /* Prepare init packet header */ 
   initPacket.header.sequence = tcpClient.sequence = 0;
   initPacket.header.length = htons(sizeof(struct initCtoS) + initPacket.handleLength);
   initPacket.header.flag = 1;

   /* Create the packet */ 
   packet = (char *) malloc(initPacket.header.length);

   /* Copy header, handle length and handle to packet */ 
   memcpy(packet, &initPacket, sizeof(struct initCtoS));
   memcpy(packet + sizeof(struct initCtoS), argv[1], initPacket.handleLength);

   /* now send the data */
   sent = send(tcpClient.socketNum, packet, initPacket.header.length, 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }

   /* Block until reply then read it into buffer*/ 
   if ((responseLength = recv(tcpClient.socketNum, buffer, BUFFER_SIZE, 0)) < 0) {
      perror("No response from server after init\n");
      exit(-1);
   }
   
   printf("Init reply length: %d\n", responseLength);
}   

int tcp_send_setup(char *host_name, char *port)
{
   int socket_num;
   struct sockaddr_in remote;       // socket address for remote side
   struct hostent *hp;              // address of remote host

   // create the socket
   if ((socket_num = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket call");
      exit(-1);
   }


   // designate the addressing family
   remote.sin_family= AF_INET;

   // get the address of the remote host and store
   if ((hp = gethostbyname(host_name)) == NULL)
   {
      printf("Error getting hostname: %s\n", host_name);
      exit(-1);
   }

   memcpy((char*)&remote.sin_addr, (char*)hp->h_addr, hp->h_length);

   // get the port used on the remote side and store
   remote.sin_port= htons(atoi(port));

   if(connect(socket_num, (struct sockaddr*)&remote, sizeof(struct sockaddr_in)) < 0)
   {
      perror("connect call");
      exit(-1);
   }

   return socket_num;
}

