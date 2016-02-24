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
Window window;

int main(int argc, char *argv[]) {
   validateParams(argc, argv);

   initRCopy(argc, argv);

   sendFile(argc, argv);

   return 0;
}

void initRCopy(int argc, char *argv[]) {
   /* Initialize sendtoErr */
   sendtoErr_init(atof(argv[4]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_OFF);

   /* Init sequence number */
   rcopy.sequence = START_SEQ_NUM;

   rcopy.bufferSize = atoi(argv[3]);
   rcopy.windowSize = atoi(argv[5]);

   /* Setup Window */ 
   window.bufferHead = NULL;
   window.bottom = START_SEQ_NUM;
   window.lower = START_SEQ_NUM;
   window.upper = START_SEQ_NUM + rcopy.windowSize;
}

void sendFile(int argc, char *argv[]) {

   int32_t select_count = 0;
   STATE state = FILENAME;

   while (state != DONE) {

      switch (state) {
         case FILENAME: 
            printf("\nSTATE = FILENAME\n");

            /* Setup new socket */
            if (udp_client_setup(argv[6], atoi(argv[7])) < 0) {
               printf("Host not found\n");
               exit(-1);
            }

            state = filename(argv[1], argv[2], atoi(argv[3]));

            if (state == FILENAME)
               close(server.sk_num);

            select_count++;
            if (select_count > 9) {
               printf("Server unreachable, client terminating\n");
               state = DONE;
            }

            break;
         case WINDOW_OPEN:
            printf("\nSTATE = WINDOW_OPEN\n");

            state = window_open();

            break;
         case WINDOW_CLOSED: 
            printf("\nSTATE = WINDOW_CLOSED\n");

            state = window_closed();

            break;
         case SEND_EOF: 
            printf("\nSTATE = SEND_EOF\n");
            
            state = send_eof();

            break;
         default: 
            printf("Error - in default state\n");
            break;
      }
   }
}

STATE filename(char *localFilename, char *remoteFilename, int32_t buf_size) {

   uint8_t packet[MAX_LEN];
   uint8_t buf[MAX_LEN];
   uint8_t flag = 0;
   int32_t seq_num = 0;
   int32_t fname_len = strlen(remoteFilename) + 1;
   int32_t recv_check = 0;

   /* Open the local file for reading */ 
   if ((rcopy.localFile = open(localFilename, O_RDONLY)) < 0) {
      printf("Error opening local file \n");
      return DONE;
   }

   /* Send Filename to server */ 
   memcpy(buf, &buf_size, 4);
   memcpy(&buf[4], remoteFilename, fname_len);

   send_buf(buf, fname_len + 4, &server, FNAME, 0, packet);

   if (select_call(server.sk_num, 1, 0, NOT_NULL) == 1) {

      recv_check = recv_buf(packet, 1000, server.sk_num, &server, &flag, &seq_num);

      if (recv_check == CRC_ERROR) {
         printf("CRC_ERROR in filename\n");
         return FILENAME;
      }
      if (flag == FNAME_BAD) {
         printf("Bad remote fiename: %s\n", remoteFilename);
         return DONE;
      }

      return WINDOW_OPEN;
   }

   return FILENAME;
}

int receivedFinalRR = 0;
int finalPacketNumber = -1;

STATE window_open() {

   uint8_t buffer[MAX_LEN];
   uint8_t packet[MAX_LEN];
   int32_t lengthRead = 0;
   int32_t packetLength = 0;

   printWindow(window);
   
   while (window.lower < window.upper) { 
      lengthRead = read(rcopy.localFile, buffer, rcopy.bufferSize);

      if (lengthRead == -1) {
         /* Error reading */ 

         perror("send_data read error");
         return DONE;
      }
      else if (lengthRead == 0) {
         /* No more to read */

         if (finalPacketNumber == -1) {
            printf("Set final packet number to %d \n", rcopy.sequence - 1);
            finalPacketNumber = rcopy.sequence - 1;
         }
         if (receivedFinalRR) { 
            return SEND_EOF;
         }
         if (finalPacketNumber != -1 && window.lower > finalPacketNumber) {
            /* Sent all packets */

            return WINDOW_CLOSED;
         }
      }
      else {
         if (lengthRead < rcopy.bufferSize) {
            /* Last packet */ 
            finalPacketNumber = rcopy.sequence;
         }

         /* Add the new data to window */ 
         addWindowNode(&window.bufferHead, buffer, lengthRead, rcopy.sequence);
         printf("Added new window node %d\n", rcopy.sequence);

         /* Send data */ 
         buffer[lengthRead] = '\0';
         printf("Sending data packet. Length: %d. Sequence: %d\n", 
               lengthRead, rcopy.sequence);
         
         packetLength = send_buf(buffer, lengthRead, &server, DATA, 
               rcopy.sequence, packet);

         rcopy.sequence++;
         window.lower = rcopy.sequence;

         printWindow(window);
      }

      checkAndProcessAcks();
   }

   return WINDOW_CLOSED;
}

STATE window_closed() {
   /* 1 second blocking select */ 
   int attempts = 0;
   int32_t packetLength = 0;
   uint8_t packet[MAX_LEN];

   while (attempts++ < 10) {
      if (select_call(server.sk_num, 1, 0, NOT_NULL)) {
         /* Received something. Process it*/ 
         printf("Received Something. Processing it\n");

         processAck();

         return WINDOW_OPEN;
      }
      else {
         /* Timeout - resend the lowest packet */ 

         WindowNode *lowestPacket = window.bufferHead;

         packetLength = send_buf(lowestPacket->data, lowestPacket->length, &server, DATA, 
               lowestPacket->index, packet);

         printf("Attempt %d. Resent packet %d. Length: %d\n", attempts, lowestPacket->index, packetLength);
      }
   }

   printf("\nServer Hasn't respond after 10 attempts :( DONE\n");

   return DONE;
}

STATE send_eof() {
   uint8_t buffer[MAX_LEN];
   uint8_t packet[MAX_LEN];
   int32_t packetLength = 0;
   uint8_t flag = 0;
   int32_t seq_num = 0;
   int32_t recv_check = 0;
   int attempts = 0;
   
   packetLength = send_buf(buffer, 1, &server, END_OF_FILE, 
                  rcopy.sequence++, packet);

   while (attempts++ < 10) {
      if (select_call(server.sk_num, 1, 0, NOT_NULL)) {
         /* Received something. Process it*/ 
         
         recv_check = recv_buf(packet, 1000, server.sk_num, &server, &flag, &seq_num);

         if (recv_check == CRC_ERROR) {
            printf("*** CRC_ERROR in send eof***\n");
         }
         else if (flag == ACK_EOF) {
            printf("Received ACK_EOF. Sending FINAL_OK. DONE!\n");
            
            send_buf(buffer, 1, &server, FINAL_OK, 
                  rcopy.sequence++, packet);
            
            return DONE;
         }
      }
      else {
         /* Timeout - resend the lowest packet */ 

         printf("Attempt %d. Resending EOF Packet\n", attempts);
         packetLength = send_buf(buffer, 1, &server, END_OF_FILE, 
                  rcopy.sequence++, packet);
      }
   }

   return DONE;
}

void checkAndProcessAcks() {

   while (select_call(server.sk_num, 0, 0, NOT_NULL)) { 
      processAck();
   }
}

void processAck() {
   uint8_t packet[MAX_LEN];
   uint8_t flag = 0;
   int32_t seq_num = 0;
   int32_t recv_check = 0;
   int32_t rrVal, srejVal;
   
   recv_check = recv_buf(packet, 1000, server.sk_num, &server, &flag, &seq_num);

   if (recv_check == CRC_ERROR) {
      printf("*** CRC_ERROR in process acks. Ignoring it ***\n");

      return;
   }

   switch (flag) {
      case RR:
         rrVal = *((int32_t *) packet);
         printf("Received RR. Val: %d\n", rrVal);

         if (rrVal <= window.bottom) {
            printf("Received invalid RR %d\n", rrVal);
            return;
         }

         /* Update the window */ 
         removeWindowNodes(&window.bufferHead, rrVal);
         window.bottom = rrVal;
         window.upper = rrVal + rcopy.windowSize;

         printWindow(window);

         if (finalPacketNumber != -1 && rrVal == finalPacketNumber + 1) {
            printf("Recieved final RR. Setting var\n");
            receivedFinalRR = 1;
         }

         break;
      case SREJ: 
         srejVal = *((int32_t *) packet);
         printf("Received SREJ. Val: %d\n", srejVal);

         if (srejVal < window.bottom) {
            printf("Received invalid SREJ %d\n", srejVal);
            return;
         }

         /* Update the window */ 
         removeWindowNodes(&window.bufferHead, srejVal);
         window.bottom = srejVal;
         window.upper = srejVal + rcopy.windowSize;

         printWindow(window);

         /* Send data */ 
         WindowNode *resendNode = getWindowNode(&window.bufferHead, srejVal);

         printf("Sending SREJ data packet. Length: %d. Sequence: %d\n", 
               resendNode->length, resendNode->index);
         
         send_buf(resendNode->data, resendNode->length, &server, DATA, 
               resendNode->index, packet);

         break;
      default:
         printf("Defulat of process ack - shouldnt be here \n");

         break;
   }
}

int udp_client_setup(char *hostname, uint16_t port_num) {
   struct hostent *hp = NULL; // address of remote host

   server.sk_num = 0;
   server.len = sizeof(struct sockaddr_in);

   if ((server.sk_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("udp_client_setup, socket");
      exit(-1);
   }

   server.remote.sin_family = AF_INET;

   hp = gethostbyname(hostname);

   if (hp == NULL) {
      printf("Host not found: %s\n", hostname);
      return -1;
   }

   memcpy(&(server.remote.sin_addr), hp->h_addr, hp->h_length);

   server.remote.sin_port = htons(port_num);

   return 0;
}

void validateParams(int argc, char *argv[]) {

   if (argc != 8) {
      printf("Usage: %s local-file remote-file buffer-size error-percent "
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
