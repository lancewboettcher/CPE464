typedef enum State STATE;

enum State {
   DONE, FILENAME, RECV, FILE_OK
};

void validateParams(int argc, char *argv[]);
void initRCopy(int argc, char *argv[]);
void sendFile(int argc, char *argv[]);
int udp_send_setup(char *host_name, int port);
STATE filename(char *fname, int32_t buf_size);

struct rcopy {

   int sequence;

};
