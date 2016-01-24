
#define MAX_HANDLE_LENGTH 255
#define MAX_SERVER_NAME_LENGTH 255
#define BUFFER_SIZE 1024

int validateParams(int argc, char *argv[]);
void initClient(char *argv[]);
int tcp_send_setup(char *host_name, char *port);

struct tcpClient {
   char handle[MAX_HANDLE_LENGTH];
   int socketNum;
   int sequence;
};
