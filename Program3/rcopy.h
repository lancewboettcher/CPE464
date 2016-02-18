void validateParams(int argc, char *argv[]);
void initRCopy(int argc, char *argv[]);
void sendFile();
int tcp_send_setup(char *host_name, uint16_t *port);

struct rcopy {

   int serverSocket;
   int clientSocket;

};
