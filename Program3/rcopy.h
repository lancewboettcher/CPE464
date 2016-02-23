typedef enum State STATE;

enum State {
   DONE, FILENAME, WINDOW_OPEN, WINDOW_CLOSED, SEND_EOF
};

void validateParams(int argc, char *argv[]);
void initRCopy(int argc, char *argv[]);
void sendFile(int argc, char *argv[]);
int udp_client_setup(char *hostname, uint16_t port_num);
STATE filename(char *localFilename, char *remoteFilename, int32_t buf_size);
STATE window_open();
STATE window_closed();
STATE send_eof();
void checkAndProcessAcks();
void processAck();

struct rcopy {
   int sequence;
   int localFile;
   int bufferSize;
   int windowSize;
};

