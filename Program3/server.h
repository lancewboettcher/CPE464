/* 
 * server.h
 *
 * CPE 464 
 * Lance Boettcher
 */
typedef enum State STATE;

enum State {
   START, DONE, FILENAME, RECV_DATA, RECVD_EOF 
};

void initServer(int argc, char *argv[]);
void runServer();
void processClient(uint8_t *buf, int32_t recv_len, Connection *client);
STATE filename(Connection *client, uint8_t *buf, int32_t recv_len, 
      int32_t *data_file, int32_t *buf_size);
STATE recv_data(int32_t output_file, Connection *client);
STATE recvd_eof(Connection *client);
int udp_recv_setup(int portNumber);

struct server {

   int serverSocket;
   int sequence;
   int clientSocket;

};
