/******************************************************************************
 * tcp_server.c
 *
 * CPE 464 - Program 1
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

struct tcpServer tcpServer;

int main(int argc, char *argv[]) {
   initServer(argc, argv);

   runServer();

   /*
    int server_socket= 0;   //socket descriptor for the server socket
    int client_socket= 0;   //socket descriptor for the client socket
    char *buf;              //buffer for receiving from client
    int buffer_size= 1024;  //packet size variable
    int message_len= 0;     //length of the received message

    printf("sockaddr: %d sockaddr_in %d\n", sizeof(struct sockaddr), sizeof(struct sockaddr_in));
    
    //create packet buffer
    buf=  (char *) malloc(buffer_size);

    //create the server socket
    server_socket= tcp_recv_setup(0);



    //look for a client to serve
    client_socket= tcp_listen(server_socket, 5);

    //now get the data on the client_socket
    if ((message_len = recv(client_socket, buf, buffer_size, 0)) < 0)
      {
	perror("recv call");
	exit(-1);
      }

    printf("Message received, length: %d\n", message_len);
    printf("Data: %s\n", buf);
    
    close(server_socket);
    close(client_socket);
*/
    return 0;
}

void initServer(int argc, char *argv[]) {
   /* Create server Socket */
   if (argc > 1) 
      tcpServer.serverSocket = tcp_recv_setup(atoi(argv[1]));
   else 
      tcpServer.serverSocket = tcp_recv_setup(0);

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
      if (select(max + 1 , &tcpServer.openFds , NULL , NULL , NULL) < 0) {
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

         printf("Added new client: %d \n", newClient);
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

   printf("Handling Client: %d\n", activeClient);

   if ((messageLength = recv(activeClient, buffer, BUFFER_SIZE, 0)) < 0) {
      perror("Error reading active client \n");
      exit(-1);
   }

   printf("Message recieved\n");

   /* Client disconnected */ 
   if (messageLength == 0) 
      removeClient(activeClient);

   else {
      /* Read message */  
      printf("Message received, length: %d\n", messageLength);
      printf("Data: %s\n", buffer);

      struct header *header = (struct header *)buffer;
      printf("Flag: %u\n", header->flag);

      switch(header->flag) {
         case 1: 
            handleClientInit(activeClient, buffer);
            break;
         case 4: 

            break;
         case 5: 

            break;

         case 8:

            break;

         case 10: 

            break;
         default: 
            printf("Unknown flag recieved: %u\n", header->flag);
      }
   }
}

void handleClientInit(int socket, char *packet) {
   struct initCtoS *initPacket = (struct initCtoS *) packet;
   char handle[MAX_HANDLE_LENGTH + 1];
   int sent;

   memcpy(handle, &initPacket->handleLength + 1, 
         initPacket->handleLength);
   handle[initPacket->handleLength] = '\0';

   printf("Handling init from socket: %d, handle: %s\n", socket, handle);

   if (existingHandle(handle)) {
      
   }
   else {
      /* Valid handle, update list and send approval to client */ 
      setHandle(socket, handle);
      
      struct header *ackPacket = (struct header *) malloc(sizeof(struct header));
      ackPacket->sequence = 1;
      ackPacket->length = htons(sizeof(struct header));
      ackPacket->flag = 2;

      sent = send(socket, ackPacket, ntohs(ackPacket->length), 0);

      if (sent < 0) {
         printf("Error sending ack init \n");
      }
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

void addClient(int socket) {
   printf("Adding client with socket: %d\n", socket);

   struct client *newClient = (struct client *) malloc(sizeof(struct client));
   newClient->socket = socket;
   newClient->next = NULL;

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
   printf("Removing client: %d\n", socket);

   struct client *iterator = tcpServer.clientList;

   if (iterator->next == NULL) {
      if (iterator->socket == socket)
         tcpServer.clientList = NULL;
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

    printf("socket has port %d \n", ntohs(local.sin_port));
	        
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

