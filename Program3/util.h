#define MAX_LEN 1500
#define START_SEQ_NUM 1

enum FLAG {
   FNAME, DATA, FNAME_OK, FNAME_BAD, RR, SREJ, END_OF_FILE, CRC_ERROR = -1
};

enum SELECT {
   SET_NULL, NOT_NULL
};

typedef struct connection Connection;

struct connection {
   int32_t sk_num;
   struct sockaddr_in remote;
   uint32_t len;
};

int32_t send_buf(uint8_t *buf, uint32_t len, Connection *connection, 
      uint8_t flag, uint32_t seq_num, uint8_t *packet);

int32_t recv_buf(uint8_t *buf, int32_t len, int32_t recv_sk_num, 
      Connection *connection, uint8_t *flag, int32_t *seq_num);

int32_t select_call(int32_t socket_num, int32_t seconds, int32_t microseconds, 
      int32_t set_null);
