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

#include "util.h"
#include "server.h"
#include "cpe464.h"

struct server server;

int main(int argc, char *argv[]) {
   initServer(argc, argv);

   runServer();

   return 0;
}

void initServer(int argc, char *argv[]) {
   /* Initialize sendtoErr */ 
   if (argc >= 2) // Error Percent
      sendtoErr_init(atof(argv[1]), DROP_OFF, FLIP_OFF, DEBUG_OFF, RSEED_OFF); 
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
   pid_t pid = 0;
   //int status = 0;
   uint8_t buf[MAX_LEN];
   Connection client;
   uint8_t flag = 0;
   int32_t seq_num = 0;
   int32_t recv_len = 0;

   while (1) {
      if (select_call(server.serverSocket, 1, 0, NOT_NULL) == 1) {
         printf("New Client\n");

         recv_len = recv_buf(buf, 1000, server.serverSocket, &client, &flag, &seq_num);

         if (recv_len != CRC_ERROR) {
            /*   if((pid = fork()) < 0) {
                 perror("fork");
                 exit(-1);
                 }
                 */
            if (pid == 0) {
               processClient(buf, recv_len, &client);
               exit(0);
            }
         }
         /*
            while (waitpid(-1, &status, WNOHANG) > 0) {
            printf("processed wait\n");
            }*/
      }

   }

}

void processClient(uint8_t *buf, int32_t recv_len, Connection *client) {

   STATE state = START;
   int32_t data_file = 0;
   //int32_t packet_len = 0;
   //uint8_t packet[MAX_LEN];
   int32_t buf_size = 0;
   int32_t seq_num = START_SEQ_NUM;

   while (state != DONE) {
      switch(state) {
         case START:
            state = FILENAME;
            break;

         case FILENAME:
            printf("\nSTATE = FILENAME\n");
            seq_num = 1;
            state = filename(client, buf, recv_len, &data_file, &buf_size);
            break;

         case RECV_DATA:
            printf("\nSTATE = RECV_DATA\n");
            //state = recv_data(data_file, client);
            state = DONE;
            break;

         case DONE:
            printf("\nSTATE = DONE\n");
            break;

         default:
            printf("In default server, shouldn't have gotten here\n");
            state = DONE;
            break;
      }
   }
}

STATE filename(Connection *client, uint8_t *buf, int32_t recv_len, 
      int32_t *data_file, int32_t *buf_size) {

   uint8_t response[1];
   char fname[MAX_LEN];

   memcpy(buf_size, buf, 4);
   memcpy(fname, &buf[4], recv_len - 4);

   if ((client->sk_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("filename, open client socket");
      exit(-1);
   }

   if (((*data_file) = open(fname, O_RDWR)) < 0) {
      printf("Error opening file\n");

      send_buf(response, 0, client, FNAME_BAD, 0, buf);
      return DONE;
   } else {
      printf("Succesfully opened file\n");

      send_buf(response, 0, client, FNAME_OK, 0, buf);
      return RECV_DATA;
   }

   return DONE;
}

STATE recv_data(int32_t output_file, Connection *client) {
   int32_t seq_num = 0;
   uint8_t flag = 0;
   int32_t data_len = 0;
   uint8_t data_buf[MAX_LEN];
   uint8_t packet[MAX_LEN];
   static int32_t expected_seq_num = START_SEQ_NUM;

   if (select_call(client->sk_num, 10, 0, NOT_NULL) == 0) {
      printf("Timeout after 10 s, client done\n");
      return DONE;
   }

   data_len = recv_buf(data_buf, 1400, client->sk_num, client, &flag, &seq_num);

   if (data_len == CRC_ERROR) {
      return RECV_DATA;
   }

   send_buf(packet, 1, client, ACK, 0, packet);

   if (flag == END_OF_FILE) {
      printf("file done\n");
      return DONE;
   }

   if (seq_num == expected_seq_num) {
      expected_seq_num++;
      write(output_file, &data_buf, data_len);
   }

   return RECV_DATA;
}

int32_t udp_recv_setup(int portNumber) {
   int sk = 0; 
   struct sockaddr_in local; 
   uint32_t len = sizeof(local); 

   if ((sk = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("Socket");
      exit(-1);
   }

   local.sin_family = AF_INET; 
   local.sin_addr.s_addr = INADDR_ANY; 
   local.sin_port = htons(portNumber);

   if (bindMod(sk, (struct sockaddr *)&local, sizeof(local)) < 0) {
      perror("udp_server, bind");
      exit(-1);
   }

   getsockname(sk, (struct sockaddr *)&local, &len);
   printf("Using Port # %d\n", ntohs(local.sin_port));

   return sk;
}
