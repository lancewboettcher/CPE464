/******************************************************************************
 * tcp_client.c
 * CPE 464 
 * Lance Boettcher 
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

int main(int argc, char * argv[]) {
   /* check command line arguments  */
   if (validateParams(argc, argv) != 0) {
      printf("usage: %s handle host-name port-number \n", argv[0]);
      exit(-1);
   }

   initClient(argv);

   runClient();

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
   initPacket.header.sequence = tcpClient.sequence = 1;
   initPacket.header.length = htons(sizeof(struct initCtoS) + initPacket.handleLength);
   initPacket.header.flag = 1;

   /* Create the packet */ 
   packet = (char *) malloc(initPacket.header.length);

   /* Copy header, handle length and handle to packet */ 
   memcpy(packet, &initPacket, sizeof(struct initCtoS));
   memcpy(packet + sizeof(struct initCtoS), argv[1], initPacket.handleLength);

   /* now send the data */
   sent = send(tcpClient.socketNum, packet, ntohs(initPacket.header.length), 0);
   if (sent < 0) {
      perror("send call");
      exit(-1);
   }

   /* Block until reply then read it into buffer */
   if ((responseLength = recv(tcpClient.socketNum, buffer, BUFFER_SIZE, 0)) < 0) {
      perror("No response from server after init\n");
      exit(-1);
   }
   
   struct header *initReply = (struct header *) buffer;
   
   if (initReply->flag == 3) {
      /* Handle in use */
      printf("Handle already in use: %s\n", tcpClient.handle);
      exit(-1);
   }
   else if (initReply->flag == 2) {
      /* Valid handle */ 
   }
   else {
      perror("Init reply returned unknown flag\n");
      exit(-1);
   }

   printf("$:");
   fflush(stdout);
}   

void runClient() {
   while (1) {
      FD_ZERO(&tcpClient.openFds);
      FD_SET(tcpClient.socketNum, &tcpClient.openFds);

      /* Set stdin */ 
      FD_SET(0, &tcpClient.openFds);

      if (select(FD_SETSIZE, &tcpClient.openFds, NULL, NULL, NULL) < 0) {
         perror("Error with client select\n");
         exit(-1);
      }
      
      if (FD_ISSET(tcpClient.socketNum, &tcpClient.openFds)) {
         /* Activity from server */ 
        
         handleServerActivity(); 
      }
      else if (FD_ISSET(0, &tcpClient.openFds)) {
         /* Activity from keyboard */ 

         handleKeyboardInput();
      }
   }
}

void handleServerActivity() {
   int messageLength;
   char buffer[BUFFER_SIZE];

   if ((messageLength = recv(tcpClient.socketNum, buffer, BUFFER_SIZE, 0)) < 0) {
      perror("Error receiving from active server\n");
      exit(-1);
   }

   if (messageLength == 0) { 
      printf("Server Terminated\n");
      exit(-1);
   }

   else {
      struct header *header = (struct header *) buffer;
      
      switch (header->flag) {
         case 4: 
            /* Broadcast */ 
            handleBroadcast(buffer, messageLength);

            break;
         case 5: 
            /* Message */ 
            handleMessage(buffer, messageLength);

            break;
         case 6: 
            /* Ack valid message */ 
            ackValidMessage(buffer);
            
            break;
         case 7: 
            /* Ack error message */ 
            ackErrorMessage(buffer);

            break;
         case 9: 
            /* Ack client exit */ 
            ackExit(buffer);

            break;
         case 11: 
            /* Num handles */ 
            numHandlesResponse(buffer);

            break;
         case 12: 
            /* Handles */ 
            handlesResponse(buffer, messageLength);

            break;
      }   
   }
}

void handleKeyboardInput() {
   char buffer[MAX_MESSAGE_LENGTH];
   int inputLength;
   
   inputLength = 0;
   while ((buffer[inputLength] = getchar()) != '\n' && inputLength < MAX_MESSAGE_LENGTH)
      inputLength++;

   buffer[inputLength] = '\0';

   if (buffer[0] != '%') {
      printf("Invalid Command\n");

      printf("$:");
      fflush(stdout);
   }
   else {
      switch (buffer[1]) {
         case 'M': /* Message */ 
         case 'm': 
            sendMessage(buffer);

            break;
         case 'B': /* Broadcast */ 
         case 'b': 
            sendBroadcast(buffer);

            break;
         case 'L': /* List handles */
         case 'l':
            listHandles();

            break;
         case 'E': /* Exit */
         case 'e':
            exitClient();

            break;
         default: 
            printf("Invalid Command\n");
      }
   }
}

/* Client to server handlers */ 

void sendMessage(char *userInput) {
   /* Skip %M */ 
   char *handle = userInput + 3;
   char *message = handle;

   /* No handle given */ 
   if (*(handle - 1) == '\0' || *handle == '\0') {
      printf("Error, no handle given\n");

      printf("$:");
      fflush(stdout);

      return;
   }

   while (*message != '\0' && *message != ' ')
      message++;

   if (*message == '\0') {
      message = " ";
   }
   else { 
      /* Replace space with NULL to split handle and message */  
      *message++ = '\0';
   }

   /* Make sure message isnt too long */
   int messageLength = strlen(message); 
   if (messageLength > MAX_MESSAGE_LENGTH) {
      printf("\nMessage is %d bytes, this is too long. Message truncated to 32kbytes\n", 
            messageLength);
      message[MAX_MESSAGE_LENGTH] = '\0';
   }

   createAndSendMessagePacket(handle, message, messageLength);

   printf("$:");
   fflush(stdout);
}

void createAndSendMessagePacket(char *handle, char *message, int messageLength) {
   struct header header;
   header.sequence = tcpClient.sequence++;
   header.length = htons(sizeof(struct header) + strlen(handle) + 
      messageLength + strlen(tcpClient.handle) + 3);
   header.flag = 5;

   char *packetHead = malloc(ntohs(header.length));
   char *packet = packetHead;

   /* Copy the header */ 
   memcpy(packet, &header, sizeof(header));
   packet += sizeof(header);

   /* Copy the dest handle length and dest handle (no nulls) */ 
   *packet++ = strlen(handle);
   memcpy(packet, handle, strlen(handle));
   packet += strlen(handle);

   /* Copy the src handle length and handle (no nulls) */ 
   *packet++ = strlen(tcpClient.handle);
   memcpy(packet, &tcpClient.handle, strlen(tcpClient.handle));
   packet += strlen(tcpClient.handle);

   /* Copy the message */ 
   memcpy(packet, message, messageLength + 1);

   int sent = send(tcpClient.socketNum, packetHead, ntohs(header.length), 0);
   if (sent < 0) {
      perror("Error sending message packet to server\n");
      exit(-1);
   }
}

void sendBroadcast(char *buffer) {
   /* Skip %B */ 
   char *message = buffer + 2;

   if (strlen(message) == 0) {
      /* If no message, send a blank space */ 

      message = " ";
   }
   else {
      /* Replace space with NULL */  
      *message++ = '\0';
   }

   struct header header;
   header.sequence = tcpClient.sequence++;
   header.length = htons(sizeof(struct header) + strlen(message) + 
         strlen(tcpClient.handle) + 2);
   header.flag = 4;

   char *packet = malloc(ntohs(header.length));
   char *packetIter = packet;

   /* Copy the header */ 
   memcpy(packetIter, &header, sizeof(header));
   packetIter += sizeof(header);

   /* Copy the src handle length and handle (no nulls) */ 
   *packetIter++ = strlen(tcpClient.handle);
   memcpy(packetIter, &tcpClient.handle, strlen(tcpClient.handle));
   packetIter += strlen(tcpClient.handle);

   /* Copy the message */ 
   memcpy(packetIter, message, strlen(message) + 1);

   /* now send the data */
   int sent = send(tcpClient.socketNum, packet, ntohs(header.length), 0);
   if (sent < 0) {
      perror("Error sending message packet to server\n");
      exit(-1);
   }

   printf("$:");
   fflush(stdout);
}

void listHandles() {
   struct header *header = (struct header *) malloc(sizeof(struct header));

   header->sequence = tcpClient.sequence++;
   header->length = htons(sizeof(struct header));
   header->flag = 10; 

   int sent = send(tcpClient.socketNum, header, ntohs(header->length), 0);
   if (sent < 0) {
      perror("Error sending exit packet to server\n");
      exit(-1);
   }
}

void exitClient() {
   struct header *header = (struct header *) malloc(sizeof(struct header));

   header->sequence = tcpClient.sequence++;
   header->length = htons(sizeof(struct header));
   header->flag = 8; 

   int sent = send(tcpClient.socketNum, header, ntohs(header->length), 0);
   if (sent < 0) {
      perror("Error sending exit packet to server\n");
      exit(-1);
   }
}

/* Server to client handlers */

void handleBroadcast(char *packet, int lengthReceived) {
   char *packetIter = packet;
   uint8_t srcHandleLength;
   char srcHandle[MAX_HANDLE_LENGTH];

   struct header *packetHeader = (struct header *) packet;
   
   /* Skip header */ 
   packetIter += sizeof(struct header);

   /* Get the source handle */ 
   srcHandleLength = *packetIter++;
   memcpy(srcHandle, packetIter, srcHandleLength);
   srcHandle[srcHandleLength] = '\0';
   packetIter += srcHandleLength;

   /* Get the message */
   int firstMessageLength = lengthReceived - sizeof(struct header) - 
      srcHandleLength - 1;
   int lengthRemaining = ntohs(packetHeader->length) - lengthReceived;
   
   printMessage(packetIter, lengthReceived, lengthRemaining, firstMessageLength, srcHandle); 
}

void handleMessage(char *packet, int lengthReceived) {
   char *packetIter = packet;
   uint8_t destHandleLength, srcHandleLength;
   char srcHandle[MAX_HANDLE_LENGTH];

   struct header *packetHeader = (struct header *) packet;

   packetIter += sizeof(struct header);
   
   /* Skip past dest handle */ 
   destHandleLength = *packetIter++;
   packetIter += destHandleLength;

   /* Get the source handle */ 
   srcHandleLength = *packetIter++;
   memcpy(srcHandle, packetIter, srcHandleLength);
   srcHandle[srcHandleLength] = '\0';
   packetIter += srcHandleLength;

   /* Get the message */
   int firstMessageLength = lengthReceived - sizeof(struct header) - 
      destHandleLength - srcHandleLength - 2;
   int lengthRemaining = ntohs(packetHeader->length) - lengthReceived;
   
   printMessage(packetIter, lengthReceived, lengthRemaining, 
         firstMessageLength, srcHandle); 
}

void printMessage(char *packetIter, int lengthReceived, int lengthRemaining, 
      int firstMessageLength, char *srcHandle) {

   char message[MAX_MESSAGE_LENGTH];
   int messageLength, thisMessageLength;
   char *buffer[BUFFER_SIZE];

   thisMessageLength = firstMessageLength;

   memcpy(message, packetIter, thisMessageLength);
 
   while (lengthRemaining > 0) {

      if ((messageLength = recv(tcpClient.socketNum, buffer, BUFFER_SIZE, 0)) < 0) {
         perror("Error recieving from active server\n");
         exit(-1);
      }

      if (messageLength == 0) { 
         printf("Server Terminated\n");
         exit(-1);
      }

      memcpy(message, buffer, messageLength);
      lengthRemaining -= messageLength;
   }

   printf("\n%s: %s\n", srcHandle, message);
   printf("$:");
   fflush(stdout);
}

void ackValidMessage(char *packet) {
   /* Nothing to do here */ 
}

void ackErrorMessage(char *packet) {
   char *packetIter = packet;
   uint8_t handleLength;
   char handle[MAX_HANDLE_LENGTH];

   packetIter += sizeof(struct header);
   handleLength = *packetIter++;

   memcpy(handle, packetIter, handleLength);
   handle[handleLength] = '\0';

   printf("\nClient with handle %s does not exist\n", handle);

   printf("$:");
   fflush(stdout);
}

void ackExit(char *packet) {
   exit(0); 
}

void numHandlesResponse(char *packet) {
   char *packetIter = packet;
   packetIter += sizeof(struct header);

   tcpClient.numHandles = ntohl(*((uint32_t *) packetIter));

   printf("Num handles: %d\n", tcpClient.numHandles);
}

void handlesResponse(char *packet, int lengthReceived) {
   int i, lengthRemaining, messageLength;
   char *packetIter = packet;
   char *endOfPacket = packet + lengthReceived;
   char handleBuffer[MAX_HANDLE_LENGTH];
   uint8_t handleLength;
   char buffer[BUFFER_SIZE];

   packetIter += sizeof(struct header);

   for (i = 0; i < tcpClient.numHandles; i++) {
      /* Get next packet if we have to */
      if (packetIter == endOfPacket) {
         if ((messageLength = recv(tcpClient.socketNum, buffer, BUFFER_SIZE, 0)) < 0) {
            perror("Error recieving from active server\n");
            exit(-1);
         }
         if (messageLength == 0) { 
            printf("Server Terminated\n");
            exit(-1);
         }
         packetIter = buffer;
         endOfPacket = packetIter + messageLength;

         lengthRemaining -= messageLength;
      }
      
      handleLength = *((uint8_t *) packetIter++);

      if (packetIter + handleLength <= endOfPacket) {
         /* Copy the handle into buffer */ 
         memcpy(handleBuffer, packetIter, handleLength);

         packetIter += handleLength;
      }
      else {
         /* Handle split between packets */
         int partialLength = endOfPacket - packetIter;

         memcpy(handleBuffer, packetIter, partialLength);
         
         if ((messageLength = recv(tcpClient.socketNum, buffer, BUFFER_SIZE, 0)) < 0) {
            perror("Error recieving from active server\n");
            exit(-1);
         }

         if (messageLength == 0) { 
            printf("Server Terminated\n");
            exit(-1);
         }

         packetIter = buffer;
         endOfPacket = packetIter + messageLength;

         memcpy(handleBuffer + partialLength, packetIter, handleLength - partialLength);

         packetIter += handleLength - partialLength; 
         lengthRemaining -= messageLength;
      }

      handleBuffer[handleLength] = '\0';

      /* Print the handle */ 
      printf("%s\n", handleBuffer);

      /* Clear the buffer */ 
      memset(handleBuffer, '\0', MAX_HANDLE_LENGTH);
   }

   printf("$:");
   fflush(stdout);
}

/* Helpers */ 

int tcp_send_setup(char *host_name, char *port) {
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
