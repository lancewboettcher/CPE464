/* 
 * server.h
 *
 * CPE 464 
 * Lance Boettcher
 */

void initServer(int argc, char *argv[]);
void runServer();
int udp_recv_setup(int portNumber);

struct server {

   int serverSocket;
   int sequence;
   int clientSocket;

};
