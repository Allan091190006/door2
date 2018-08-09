#define CONFIG_FILE "QR_CONFIG.ini"
struct server_config{
int CBR_NUMBER;
int comm_number;
int channel_number;
int gSendToPort;
char gSendToIP[];
char QR_IP_HEAD[256];
char PROXY[32];
char gFromIP[32];
char Store_ID[20];
char MAC_ADDR[20];
char HrtBt_SndURL[256];
char HrtBt_URL[256];
char QR_SCAN[256];
}
typedef server_config config_t;
config_t ;
#ifdef DOOR_ALPHA
#else
#endif
