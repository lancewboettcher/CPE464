typedef enum State STATE;

enum State {
   DONE, FILENAME, SEND_DATA, PROCESS_ACKS, WINDOW_CLOSED
};

void validateParams(int argc, char *argv[]);
void initRCopy(int argc, char *argv[]);
void sendFile(int argc, char *argv[]);
int udp_client_setup(char *hostname, uint16_t port_num);
STATE filename(char *localFilename, char *remoteFilename, int32_t buf_size);
STATE send_data();
STATE processAcks();
STATE windowClosed();

struct rcopy {

   int sequence;
   int localFile;
   int bufferSize;
   int windowSize;

};
