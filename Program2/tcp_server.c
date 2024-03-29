/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
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
#include "tcp_server.h"
#include "packets.h"
#include "testing.h"

struct tcpServer tcpServer;

int main(int argc, char *argv[]) {
   initServer(argc, argv);

   runServer();

   return 0;
}

void initServer(int argc, char *argv[]) {
   /* Create server Socket */
   if (argc > 1) 
      tcpServer.serverSocket = tcp_recv_setup(atoi(argv[1]));
   else 
      tcpServer.serverSocket = tcp_recv_setup(0);
   
   tcpServer.sequence = 1;

   addClient(tcp_listen(tcpServer.serverSocket, 5));
}

void runServer() {
   int max, newClient;
   struct client *clientIterator;
   
   while (1) {
      /* Clear fds */ 
      FD_ZERO(&tcpServer.openFds);

      /* Add the server socket */ 
      FD_SET(tcpServer.serverSocket, &tcpServer.openFds);
      max = tcpServer.serverSocket;

      /* Add all the client sockets */ 
      clientIterator = tcpServer.clientList;
      while (clientIterator != NULL) {
         FD_SET(clientIterator->socket, &tcpServer.openFds);

         if (clientIterator->socket > max)
            max = clientIterator->socket;

         clientIterator = clientIterator->next;
      }   
      
      /* Wait for something to happen */ 
      if (select(max + 1, &tcpServer.openFds , NULL , NULL , NULL) < 0) {
         perror("Error with select\n");
         exit(-1);
      }

      /* If activity on server socket -> new connection */ 
      if (FD_ISSET(tcpServer.serverSocket, &tcpServer.openFds)) {

         /* Accept the new client */ 
         if ((newClient = accept(tcpServer.serverSocket,
                     (struct sockaddr*) 0, (socklen_t *) 0)) < 0) {
            perror("Error accepting new client \n");
            exit(-1);
         }

         /* Add the new client to our list */
         addClient(newClient);
      }

      /* If other client active -> handle it */ 
      clientIterator = tcpServer.clientList;
      while (clientIterator != NULL) {
         if (FD_ISSET(clientIterator->socket, &tcpServer.openFds)) {
            handleActiveClient(clientIterator->socket);
         }

         clientIterator = clientIterator->next;
      }
   }
}

void handleActiveClient(int activeClient) {
   int messageLength;
   char buffer[BUFFER_SIZE];

   if ((messageLength = recv(activeClient, buffer, BUFFER_SIZE, 0)) < 0) {
      perror("Error reading active client \n");
      exit(-1);
   }

   /* Client disconnected */ 
   if (messageLength == 0) 
      removeClient(activeClient);

   else {
      /* Read message */  
      struct header *header = (struct header *)buffer;

      switch (header->flag) {
         case 1:
            /* Client init packet */    
            handleClientInit(activeClient, buffer);
            break;
         case 4: 
            /* Broadcast packet */ 
            handleClientBroadcast(activeClient, buffer);

            break;
         case 5: 
            /* Message packet */ 
            handleClientMessage(activeClient, buffer, messageLength);

            break;
         case 8:
            /* Client exit packet */ 
            handleClientExit(activeClient, buffer);

            break;

         case 10: 
            /* Client list handle packet */ 
            handleClientListHandles(activeClient, buffer);

            break;
      }
   }
}

void handleClientInit(int socket, char *packet) {
   struct initCtoS *initPacket = (struct initCtoS *) packet;
   char handle[MAX_HANDLE_LENGTH + 1];
   int sent;
   struct header *ackPacket;

   memcpy(handle, &initPacket->handleLength + 1, 
         initPacket->handleLength);
   handle[initPacket->handleLength] = '\0';

   ackPacket = (struct header *) malloc(sizeof(struct header));
   ackPacket->sequence = tcpServer.sequence++;
   ackPacket->length = htons(sizeof(struct header));

   if (existingHandle(handle)) {
      /* Invalid Handle, send error packet back */ 
      ackPacket->flag = 3;
   }
   else {
      /* Valid handle, update list and send approval to client */

      setHandle(socket, handle);
      ackPacket->flag = 2;
   }
   
   /* Send the response packet */ 
   sent = send(socket, ackPacket, ntohs(ackPacket->length), 0);

   if (sent < 0) {
      printf("Error sending ack init \n");
   }
}   

void handleClientBroadcast(int socket, char *packet) {
   char *packetIter = packet;
   int destSocket, srcSocket, sent;
   struct client *clientIterator;
   
   struct header *packetHeader = (struct header *) packet;
   packetIter += sizeof(struct header);

   /* Get the src handle and length from the packet */ 
   uint8_t srcHandleLength = *packetIter++;
   char srcHandle[MAX_HANDLE_LENGTH];

   memcpy(srcHandle, packetIter, srcHandleLength);
   packetIter += srcHandleLength;
   srcHandle[srcHandleLength] = '\0';

   srcSocket = getClientSocket(srcHandle);

   /* Forward the packet to all clients */ 
   clientIterator = tcpServer.clientList;
   while (clientIterator != NULL) {
      destSocket = clientIterator->socket;

      /* Don't send message to source */ 
      if (destSocket == srcSocket) {
         clientIterator = clientIterator->next;
         continue; 
      }

      sent = send(destSocket, packet, ntohs(packetHeader->length), 0);

      if (sent < 0) {
         printf("Error sending broadcast \n");
      } 

      clientIterator = clientIterator->next;
   }
}

void handleClientMessage(int socket, char *packet, int lengthReceived) {
   char *packetIter = packet;
   char *responsePacket;
   struct header responseHeader;
   int validDest;

   struct header *header = (struct header *) packet;
   packetIter += sizeof(struct header);

   /* Get the dest handle and length from the packet */ 
   uint8_t destHandleLength = *packetIter++;
   char destHandle[MAX_HANDLE_LENGTH];

   memcpy(destHandle, packetIter, destHandleLength);
   packetIter += destHandleLength;
   destHandle[destHandleLength] = '\0';
   
   /* Prepare a response packet */ 
   responseHeader.sequence = tcpServer.sequence++;
   responseHeader.length = htons(sizeof(struct header) + destHandleLength + 1);
   
   if (!existingHandle(destHandle)) {
      /* handle does not exist, send flag = 7 back */
      
      responseHeader.flag = 7;
      validDest = 0;
   }
   else {
      /* valid handle, send flag = 6 back */     
      responseHeader.flag = 6;
      validDest = 1;
   }

   responsePacket = malloc(sizeof(struct header) + destHandleLength + 1);
   packetIter = responsePacket;

   /* Copy the dest handle length and handle */ 
   memcpy(responsePacket, &responseHeader, sizeof(struct header));
   packetIter += sizeof(struct header);
   *packetIter++ = destHandleLength;
   memcpy(packetIter, &destHandle, destHandleLength);

   /* Send the response packet */ 
   int sent = send(socket, responsePacket, ntohs(responseHeader.length), 0);

   if (sent < 0) {
      printf("Error sending message response \n");
   }

   if (validDest) {
      /* Forward the message to dest if valid dest handle */
      int destSocket = getClientSocket(destHandle); 
      int lengthRemaining = ntohs(header->length) - lengthReceived;
      int messageLength;
      char *buffer[BUFFER_SIZE];

      sent = send(destSocket, packet, lengthReceived, 0);

      if (sent < 0) {
         printf("Error Forwarding message\n");
      }
      
      while (lengthRemaining > 0) {
         printf("Length remaining: %d\n", lengthRemaining);

         if ((messageLength = recv(socket, buffer, BUFFER_SIZE, 0)) < 0) {
            perror("Error receiving overflow message\n");
            exit(-1);
         }
         if (messageLength == 0) {
            perror("Client terminated while receiving overflow\n");
         }

         sent = send(destSocket, buffer, messageLength, 0);

         if (sent < 0) {
            perror("Error Forwarding message \n");
         } 

         lengthRemaining -= messageLength;
      }
   }
}

void handleClientExit(int socket, char *packet) {
   int sent;

   /* Prepare the ack packet */ 
   struct header *ackPacket = (struct header *) malloc(sizeof(struct header));
   ackPacket->sequence = tcpServer.sequence++;
   ackPacket->length = htons(sizeof(struct header));
   ackPacket->flag = 9;
   
   /* Send the ack packet */ 
   sent = send(socket, ackPacket, ntohs(ackPacket->length), 0);

   if (sent < 0) {
      printf("Error sending ack init \n");
   }
}

void handleClientListHandles(int socket, char *packet) {
   struct header responseHeader;
   char *responsePacket, *packetIter;
   int sent, totalHandleLength = 0;
   uint8_t handleLength;
   struct client *clientIterator;

   /* Prepare flag = 11 packet */ 
   responseHeader.sequence = tcpServer.sequence++;
   responseHeader.length = htons(sizeof(struct header) + sizeof(uint32_t));
   responseHeader.flag = 11;

   responsePacket = malloc(ntohs(responseHeader.length));
   packetIter = responsePacket;

   /* Copy the header */ 
   memcpy(responsePacket, &responseHeader, ntohs(responseHeader.length));
   packetIter += sizeof(struct header);

   /* Copy the number of handles */ 
   *(uint32_t *)packetIter = htonl(tcpServer.numClients);
   
   /* Send the num handles packet */ 
   sent = send(socket, responsePacket, ntohs(responseHeader.length), 0);

   if (sent < 0) {
      printf("Error sending list handle number message response \n");
   } 

   /* Prepare flag = 12 packet */ 
   responseHeader.sequence = tcpServer.sequence++;
   responseHeader.length = 0; //This is supposed to be zero
   responseHeader.flag = 12;

   responsePacket = malloc(BUFFER_SIZE);
   packetIter = responsePacket;

   /* Copy the header */ 
   memcpy(responsePacket, &responseHeader, sizeof(struct header));
   packetIter += sizeof(struct header);

   /* Copy the handles and lengths */ 
   clientIterator = tcpServer.clientList;
   while (clientIterator != NULL) {
      handleLength = strlen(clientIterator->handle);
      totalHandleLength += handleLength + 1;
      
      *packetIter++ = handleLength;
      memcpy(packetIter, clientIterator->handle, handleLength);

      packetIter += handleLength;
      clientIterator = clientIterator->next;
   }

   sent = send(socket, responsePacket, sizeof(struct header) + totalHandleLength, 0);

   if (sent < 0) {
      printf("Error sending list handle handles response \n");
   } 
}

int existingHandle(char *handle) {
   struct client *clientIterator;

   clientIterator = tcpServer.clientList;
   while (clientIterator != NULL) {
      if (strcmp(clientIterator->handle, handle) == 0) {
         return 1;
      }

      clientIterator = clientIterator->next;
   }
   
   return 0;
}


void setHandle(int socket, char *handle) {
   struct client *clientIterator;

   clientIterator = tcpServer.clientList;
   while (clientIterator != NULL) {
      if (clientIterator->socket == socket) {
         strcpy(clientIterator->handle, handle);
         break;
      }
      clientIterator = clientIterator->next;
   }
}

int getClientSocket(char *handle) {
   struct client *clientIterator;

   clientIterator = tcpServer.clientList;
   while (clientIterator != NULL) {
      if (strcmp(clientIterator->handle, handle) == 0) {
         return clientIterator->socket;
      }

      clientIterator = clientIterator->next;
   }

   return -1;
}

void addClient(int socket) {
   struct client *newClient = (struct client *) malloc(sizeof(struct client));
   newClient->socket = socket;
   newClient->next = NULL;
   *newClient->handle = '\0';

   if (tcpServer.clientList == NULL)
      tcpServer.clientList = newClient;
   else {
      struct client *iterator = tcpServer.clientList;
      while (iterator->next != NULL) 
         iterator = iterator->next;

      iterator->next = newClient;
   }

   tcpServer.numClients++;
}

void removeClient(int socket) {
   struct client *iterator = tcpServer.clientList;

   if (iterator->next == NULL) {
      if (iterator->socket == socket)
         tcpServer.clientList = NULL;
   }

   else if (iterator->socket == socket) {
      /* Remove the first item in the list */

      tcpServer.clientList = iterator->next;

   }
   else {
      while (iterator->next->socket != socket)
         iterator = iterator->next;

      iterator->next = iterator->next->next;
   }
   
   close(socket);
   tcpServer.numClients--;
}

/* This function sets the server socket.  It lets the system
   determine the port number.  The function returns the server
   socket number and prints the port number to the screen.  */
int tcp_recv_setup(int portNumber)
{
    int server_socket= 0;
    struct sockaddr_in local;      /* socket address for local side  */
    socklen_t len= sizeof(local);  /* length of local address        */

    /* create the socket  */
    server_socket= socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
    {
      perror("socket call");
      exit(1);
    }

    local.sin_family= AF_INET;         //internet family
    local.sin_addr.s_addr= INADDR_ANY; //wild card machine address
    local.sin_port= htons(portNumber);             

    /* bind the name (address) to a port */
    if (bind(server_socket, (struct sockaddr *) &local, sizeof(local)) < 0)
      {
	perror("bind call");
	exit(-1);
      }
    
    //get the port name and print it out
    if (getsockname(server_socket, (struct sockaddr*)&local, &len) < 0)
      {
	perror("getsockname call");
	exit(-1);
      }

    printf("Server is using port %d\n", ntohs(local.sin_port));
	        
    return server_socket;
}

/* This function waits for a client to ask for services.  It returns
   the socket number to service the client on.    */

int tcp_listen(int server_socket, int back_log)
{
  int client_socket= 0;
  if (listen(server_socket, back_log) < 0)
    {
      perror("listen call");
      exit(-1);
    }
  
  if ((client_socket= accept(server_socket, (struct sockaddr*)0, (socklen_t *)0)) < 0)
    {
      perror("accept call");
      exit(-1);
    }
  
  return(client_socket);
}

