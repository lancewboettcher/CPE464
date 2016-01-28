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

#include <time.h>

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
   initPacket.header.sequence = tcpClient.sequence = 0;
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
   
   printf("Init reply length: %d\n", responseLength);
   struct header *initReply = (struct header *) buffer;
   printf("flag: %u\n", initReply->flag);
   
   if (initReply->flag == 3) {
      /* Handle in use */
      printf("Handle already in use: %s\n", tcpClient.handle);
      exit(-1);
   }
   else if (initReply->flag == 2) {
      printf("Valid Handle\n");
   }
   else {
      printf("Init reply returned unknown flag: %u\n", initReply->flag);
   }

   tcpClient.numMessagesQueued = 0;
   tcpClient.queueIndex = 0;
}   

void runClient() {
   while (1) {
      printf("$:");
      fflush(stdout);

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
   printf("\nHandling server activity\n");

   int messageLength;
   char buffer[BUFFER_SIZE];

   if ((messageLength = recv(tcpClient.socketNum, buffer, BUFFER_SIZE, 0)) < 0) {
      perror("Error recieving from active server\n");
      exit(-1);
   }

   if (messageLength == 0) { 
      printf("Server Terminated\n");
      exit(-1);
   }

   else {
      printf("Message recieved from server, length: %d\n", messageLength);

      struct header *header = (struct header *) buffer;
      printf("Flag: %u, totalLength: %hu\n", header->flag, ntohs(header->length));

      switch (header->flag) {
         case 4: 
            /* Broadcast */ 
            handleBroadcast(buffer);

            break;
         case 5: 
            /* Message */ 
            handleMessage(buffer);

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
            handlesResponse(buffer);

            break;
         default:
            printf("Unknown flag from server: %d\n", header->flag);
      }   
   }
}

void handleKeyboardInput() {
   printf("\nHandling keyboard input \n"); 

   char buffer[MAX_MESSAGE_LENGTH];
   int inputLength;
   
   inputLength = 0;
   while ((buffer[inputLength] = getchar()) != '\n' && inputLength < MAX_MESSAGE_LENGTH)
      inputLength++;

   buffer[inputLength] = '\0';

   printf("Client input: %s\n", buffer);

   if (buffer[0] != '%') {
      printf("Invalid Command\n");
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
   
   printf("\nSending Message\n");

   /* Skip %M */ 
   char *handle = userInput + 3;
   char *message = handle;

   while (*message != '\0' && *message != ' ')
      message++;

   /* No handle given */ 
   if (*message == '\0') {
      printf("Error, no handle given\n");
      return;
   }

   /* Replace space with NULL to split handle and message */  
   *message++ = '\0';

   /* Make sure message isnt too long */
   int messageLength = strlen(message); 
   if (messageLength > MAX_MESSAGE_LENGTH) {
      printf("Message is %d bytes, this is too long. Message truncated to 32kbytes\n", 
            messageLength);
      message[MAX_MESSAGE_LENGTH] = '\0';
   }

   printf(" Dest handle: '%s' message: '%s'\n", handle, message);

   int remainingMessageLength = messageLength, thisMessageLength; 
   char *messageIter = message;
   char messageBuffer[MAX_MESSAGE_PER_PACKET + 1];

   //TODO Maybe dont do this
   tcpClient.numMessagesQueued = 0;

   while (remainingMessageLength > 0) {
      printf("Sending message. Remaining: %d\n", remainingMessageLength);
      
      if (remainingMessageLength > MAX_MESSAGE_PER_PACKET) {
         thisMessageLength = MAX_MESSAGE_PER_PACKET;
      }
      else {
         thisMessageLength = remainingMessageLength;
      }

      memcpy(messageBuffer, messageIter, thisMessageLength);
      messageBuffer[thisMessageLength] = '\0';

      printf("Message length: %d buffer: %s\n", thisMessageLength, messageBuffer);

      queueMessagePacket(handle, messageBuffer, thisMessageLength);

      messageIter += thisMessageLength;
      remainingMessageLength -= thisMessageLength; 
   }

   /* Send the first message packet */ 
   struct header *messageHeader = (struct header *) tcpClient.messageQueue[0];
   int sent = send(tcpClient.socketNum, tcpClient.messageQueue[0], ntohs(messageHeader->length), 0);
   if (sent < 0) {
      perror("Error sending message packet to server\n");
      exit(-1);
   }

   printf("Sent a packet length: %hu flag: %u\n", ntohs(messageHeader->length), messageHeader->flag);

   tcpClient.queueIndex = 1;
  // tcpClient.numMessagesQueued;
}

void queueMessagePacket(char *handle, char *message, int messageLength) {
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

   /* Add the packet to the queue */ 
   tcpClient.messageQueue[tcpClient.numMessagesQueued] = packetHead;

   printf("added packet to queue at index %d \n", tcpClient.numMessagesQueued);
  // printf("packet: %p, queue[]: %p\n", packet, tcpClient.messageQueue[tcpClient.numMessagesQueued]);

   tcpClient.numMessagesQueued++;

   waitFor(2);
}

void sendBroadcast(char *buffer) {
   printf("Sending Broadcast\n");

   /* Skip %B */ 
   char *message = buffer + 2;

   /* Replace space with NULL */  
   *message++ = '\0';

   printf("Broadcast message: '%s'\n", message);

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

   printf("Sent message to server with length: %d\n", ntohs(header.length));
}

void listHandles() {
   printf("Listing Handles\n");

   struct header *header = (struct header *) malloc(sizeof(struct header));

   header->sequence = tcpClient.sequence++;
   header->length = htons(sizeof(struct header));
   header->flag = 10; 

   int sent = send(tcpClient.socketNum, header, ntohs(header->length), 0);
   if (sent < 0) {
      perror("Error sending exit packet to server\n");
      exit(-1);
   }

   printf("Sent list handles request to server with length: %d\n", ntohs(header->length));
}

void exitClient() {
   printf("Client Exiting\n");

   struct header *header = (struct header *) malloc(sizeof(struct header));

   header->sequence = tcpClient.sequence++;
   header->length = htons(sizeof(struct header));
   header->flag = 8; 

   int sent = send(tcpClient.socketNum, header, ntohs(header->length), 0);
   if (sent < 0) {
      perror("Error sending exit packet to server\n");
      exit(-1);
   }

   printf("Sent exit to server with length: %d\n", ntohs(header->length));
}

/* Server to client handlers */

void handleBroadcast(char *packet) {
   printf("\nRecieved broadcast from another client\n");

   char *packetIter = packet;
   uint8_t srcHandleLength;
   char srcHandle[MAX_HANDLE_LENGTH];
   char message[MAX_MESSAGE_LENGTH];

   /* Skip header */ 
   packetIter += sizeof(struct header);

   /* Get the source handle */ 
   srcHandleLength = *packetIter++;
   memcpy(srcHandle, packetIter, srcHandleLength);
   srcHandle[srcHandleLength] = '\0';
   packetIter += srcHandleLength;

   /* Get the message */ 
   strcpy(message, packetIter);
   
   printf("%s: %s\n", srcHandle, message);
}

void handleMessage(char *packet) {
   printf("\nRecieved message from another client\n");

   char *packetIter = packet;
   uint8_t destHandleLength, srcHandleLength;
   char srcHandle[MAX_HANDLE_LENGTH];
   char message[MAX_MESSAGE_LENGTH];

   /* Skip header */ 
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
   strcpy(message, packetIter);

   printf("%s: %s\n", srcHandle, message);
}

void ackValidMessage(char *packet) {
   printf("Ack valid message recieved\n");

   if (tcpClient.queueIndex < tcpClient.numMessagesQueued) {
      int index = tcpClient.queueIndex++; 

      struct header *messageHeader = (struct header *) tcpClient.messageQueue[index];
      int sent = send(tcpClient.socketNum, tcpClient.messageQueue[index], ntohs(messageHeader->length), 0);
      if (sent < 0) {
         perror("Error sending message packet to server\n");
         exit(-1);
      }
      printf("Sent message at index %d from the queue \n", index);
      printf("Length: %hu, flag: %u\n", ntohs(messageHeader->length), messageHeader->flag);
      
      if (tcpClient.queueIndex == tcpClient.numMessagesQueued) {
         tcpClient.queueIndex = 0;
         tcpClient.numMessagesQueued = 0;

         printf("Reset the queue\n");
      }
   }
}

void ackErrorMessage(char *packet) {
   printf("Ack message error recieved\n");

   char *packetIter = packet;
   uint8_t handleLength;
   char handle[MAX_HANDLE_LENGTH];

   packetIter += sizeof(struct header);
   handleLength = *packetIter++;

   memcpy(handle, packetIter, handleLength);
   handle[handleLength] = '\0';

   printf("Client with handle %s does not exist\n", handle);
}

void ackExit(char *packet) {
   printf("Ack Exit recieved\n");
  
   exit(0); 
}

void numHandlesResponse(char *packet) {
   printf("Num handles response recieved\n");

   char *packetIter = packet;
   packetIter += sizeof(struct header);

   tcpClient.numHandles = *((uint32_t *) packetIter);

   printf("Num handles: %d", tcpClient.numHandles);
}

void handlesResponse(char *packet) {
   printf("Handles response recieved\n");

   int i;
   char *packetIter = packet;
   char handleBuffer[MAX_HANDLE_LENGTH];
   uint8_t handleLength;

   packetIter += sizeof(struct header);

   for (i = 0; i < tcpClient.numHandles; i++) {
      handleLength = *((uint8_t *) packetIter++);

      /* Copy the handle into buffer */ 
      memcpy(handleBuffer, packetIter, handleLength);

      handleBuffer[handleLength] = '\0';

      /* Print the handle */ 
      printf("%s\n", handleBuffer);

      packetIter += handleLength;
      memset(handleBuffer, '\0', MAX_HANDLE_LENGTH);
   }
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


void waitFor (unsigned int secs) {
       unsigned int retTime = time(0) + secs;     // Get finishing time.
           while (time(0) < retTime);    // Loop until it arrives.
}
