/******************************************************************************
 * util.c
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
#include "cpe464.h"

int32_t send_buf(uint8_t *buf, uint32_t len, Connection *connection,
            uint8_t flag, uint32_t seq_num, uint8_t *packet) {

   int32_t send_len = 0;
   uint16_t checksum = 0;

   /* Set up the packet */ 
   if (len > 0)
      memcpy(&packet[7], buf, len);

   seq_num = htonl(seq_num);
   memcpy(&packet[0], &seq_num, 4);
   packet[6] = flag;

   memset(&packet[4], 0, 2);

   checksum = in_cksum((unsigned short *) packet, len + 8);

   memcpy(&packet[4], &checksum, 2);

   if ((send_len = sendtoErr(connection->sk_num, packet, len + 8, 0, 
               (struct sockaddr *) &(connection->remote), connection->len)) < 0) {
      perror("send_buf, sendto");
      exit(-1);
   }

   return send_len;
}

int32_t recv_buf(uint8_t *buf, int32_t len, int32_t recv_sk_num,
            Connection *connection, uint8_t *flag, int32_t *seq_num) {

   char data_buf[MAX_LEN];
   int32_t recv_len = 0;
   uint32_t remote_len = sizeof(struct sockaddr_in);

   if ((recv_len = recvfrom(recv_sk_num, data_buf, len, 0, 
               (struct sockaddr *) &(connection->remote), &remote_len)) < 0) {
      perror("recv_buf, recvfrom");
      exit(-1);
   }

   if (in_cksum((unsigned short *) data_buf, recv_len) != 0) {
      return CRC_ERROR;
   }

   *flag = data_buf[6];
   memcpy(seq_num, data_buf, 4);

   *seq_num = ntohl(*seq_num);

   if (recv_len > 7)
      memcpy(buf, &data_buf[7], recv_len - 8);
 
   connection->len = remote_len;

   return recv_len - 8;
}

int32_t select_call(int32_t socket_num, int32_t seconds, int32_t microseconds,
            int32_t set_null) {
   fd_set fdvar;
   struct timeval *timeout = NULL;

   if (set_null == NOT_NULL) {
      timeout = (struct timeval *) malloc(sizeof(struct timeval));
      timeout->tv_sec = seconds;
      timeout->tv_usec = microseconds;
   }

   FD_ZERO(&fdvar);
   FD_SET(socket_num, &fdvar);

   if (selectMod(socket_num + 1, (fd_set *) &fdvar, (fd_set *) 0, 
            (fd_set *) 0, timeout) < 0) {
      perror("select");
      exit(-1);
   }

   if (FD_ISSET(socket_num, &fdvar)) 
      return 1;
   else
      return 0;
}

void addWindowNode(WindowNode **head, uint8_t *data, int32_t length, int32_t index) {
   WindowNode *newNode = malloc(sizeof(WindowNode));
   memcpy(newNode->data, data, length);
   newNode->index = index;
   newNode->length = length;
   newNode->next = NULL;

   if (*head == NULL) {
      *head = newNode;
   }
   else {
      WindowNode *iterator = *head;

      while (iterator->next != NULL) {
         iterator = iterator->next;
      }
      iterator->next = newNode;
   }
}
   
void removeWindowNodes(WindowNode **head, int32_t rrVal) {
   WindowNode *temp;
   while ((*head)->index != rrVal - 1) {
      temp = *head;

      *head = (*head)->next;

      free(temp);
   }
}

WindowNode *getWindowNode(WindowNode **head, int32_t index) {
   WindowNode *iterator = *head;

   while (iterator != NULL && iterator->index != index) {
      iterator = iterator->next;
   }

   return iterator;
}

void printWindow(Window window) {
   printf("Printing Window...\n");
   printf("Bottom: %d. Lower: %d. Upper: %d\n", window.bottom, window.lower, window.upper);

   WindowNode *iterator = window.bufferHead;

   uint8_t printBuf[10];
   
   while (iterator != NULL) {
      iterator->data[iterator->length] = '\0';

      memcpy(printBuf, iterator->data, 5);
      printBuf[5] = '\0';

      printf("%d : %s\n", iterator->index, printBuf);

      iterator = iterator->next;
   }

   printf("\n");
}
