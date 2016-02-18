void validateParams(int argc, char *argv[]);
void initRCopy(int argc, char *argv[]);
void sendFile();
int udp_send_setup(char *host_name, int port);

struct rcopy {

   int serverSocket;
   int clientSocket;
   int sequence;

};
