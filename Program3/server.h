/* 
 * server.h
 *
 * CPE 464 
 * Lance Boettcher
 */

void initServer(int argc, char *argv[]);
void runServer();

struct server {

   int serverSocket;
   int sequencei;
   int clientSocket;

};
