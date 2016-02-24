#define MAX_LEN 1500
#define START_SEQ_NUM 1

enum FLAG {
   FNAME, DATA, FNAME_OK, FNAME_BAD, RR, SREJ, END_OF_FILE, ACK_EOF, FINAL_OK, CRC_ERROR = -1
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

typedef struct windowNode WindowNode;

struct windowNode {
   uint8_t data[MAX_LEN];
   int32_t length;
   int index;
   int sentSREJ;
   WindowNode *next;
};

typedef struct window Window;

struct window {
   WindowNode *bufferHead;

   int32_t bottom;
   int32_t lower;
   int32_t upper;
};

int32_t send_buf(uint8_t *buf, uint32_t len, Connection *connection, 
      uint8_t flag, uint32_t seq_num, uint8_t *packet);
int32_t recv_buf(uint8_t *buf, int32_t len, int32_t recv_sk_num, 
      Connection *connection, uint8_t *flag, int32_t *seq_num);
int32_t select_call(int32_t socket_num, int32_t seconds, int32_t microseconds, 
      int32_t set_null);
void addWindowNode(WindowNode **head, uint8_t *data, int32_t length, int32_t index);
void addWindowNodeAtIndex(Window *window, uint8_t *data, int32_t length, int32_t index);
void removeWindowNodes(WindowNode **head, int32_t rrVal);
WindowNode *getWindowNode(WindowNode **head, int32_t index);
int32_t getNewBottomIndex(Window window);
void printWindow(Window window);
