
#define MAX_HANDLE_LENGTH 255
#define MAX_SERVER_NAME_LENGTH 255
#define BUFFER_SIZE 1024
#define MAX_MESSAGE_LENGTH 32768
#define MAX_MESSAGE_PER_PACKET 800
#define MAX_PACKET_SIZE 1024

int validateParams(int argc, char *argv[]);
void initClient(char *argv[]);
void runClient();
void handleServerActivity();
void handleKeyboardInput();
void sendMessage(char *userInput);
void createAndSendMessagePacket(char *handle, char *message, int messageLength);
void sendBroadcast(char *buffer);
void listHandles();
void exitClient();
void handleBroadcast(char *packet);
void handleMessage(char *packet);
void ackValidMessage(char *packet);
void ackErrorMessage(char *packet);
void ackExit(char *packet);
void numHandlesResponse(char *packet);
void handlesResponse(char *packet);
int tcp_send_setup(char *host_name, char *port);

void waitFor (unsigned int secs);

struct tcpClient {
   char handle[MAX_HANDLE_LENGTH];
   int socketNum;
   int sequence;
   fd_set openFds;
   uint32_t numHandles;
};
