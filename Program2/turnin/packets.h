struct header {
   uint32_t sequence;
   uint16_t length;
   uint8_t flag;
}__attribute__((packed));

/* Flag = 1 */ 
struct initCtoS { 
   struct header header;
   uint8_t handleLength;
}__attribute__((packed));

/* Flag = 2 */ 
struct ackInitStoC {
   struct header header;
}__attribute__((packed));

/* Flag = 4 */ 
struct broadcastCtoS {
   struct header header;
   uint8_t handleLength;
   char *handle;
   char *message;
}__attribute__((packed));

/* Flag = 5 */ 
struct messageCtoS {
   struct header header;
   uint8_t destHandleLength;
   char *destHandle;
   uint8_t srcHandleLength;
   char *srcHandle;
   char *message;
}__attribute__((packed));

/* Flag = 6 */ 
struct ackMessageStoC {
   struct header header;
   uint8_t handleLength;
   char * handle;
}__attribute__((packed));

/* Flag = 7 */ 
struct errMessageStoC {
   struct header header;
   uint8_t handleLength;
   char * handle;
}__attribute__((packed));

/* Flag = 11 */ 
struct handleCountStoC {
   struct header header;
   uint32_t numHandles;
}__attribute__((packed));
   
