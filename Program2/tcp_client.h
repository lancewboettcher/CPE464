
#define MAX_HANDLE_LENGTH 255
#define MAX_SERVER_NAME_LENGTH 255
#define BUFFER_SIZE 1024
#define MAX_MESSAGE 32768

int validateParams(int argc, char *argv[]);
void initClient(char *argv[]);
void runClient();
void sendMessage(char *buffer);
void sendBroadcast(char *buffer);
void listHandles();
void exitClient();
int tcp_send_setup(char *host_name, char *port);

struct tcpClient {
   char handle[MAX_HANDLE_LENGTH];
   int socketNum;
   int sequence;
};
