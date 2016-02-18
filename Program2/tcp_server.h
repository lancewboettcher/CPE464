
#define BUFFER_SIZE 32768
#define MAX_CLIENTS 20
#define MAX_HANDLE_LENGTH 255
#define MAX_MESSAGE_LENGTH 32768

void initServer(int argc, char *argv[]);
void runServer();
int tcp_recv_setup(int portNumber);
int tcp_listen(int server_socket, int back_log);
void handleActiveClient(int activeClient);
void handleClientInit(int socket, char *packet);
void handleClientBroadcast(int socket, char *packet);
void handleClientMessage(int socket, char *packet, int lengthReceived);
void handleClientExit(int socket, char *packet);
void handleClientListHandles(int socket, char *packet);
int existingHandle(char *handle);
void setHandle(int socket, char *handle);
int getClientSocket(char *handle);
void addClient(int socket);
void removeClient(int socket);

struct client {
   int socket;
   char handle[MAX_HANDLE_LENGTH];
   struct client *next;
};

struct tcpServer {
   int serverSocket;
   struct client *clientList;
   uint32_t numClients;
   fd_set openFds;
   int sequence;
};
