#define CONFIG_FILE "QR_CONFIG.ini"
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef struct TRefreshSendT{
	char head;
	short device_num;
	char date[7];
	char MacActive[256];
	char XOR;
	char end;
}TRefreshSend;
typedef struct TNoRefreshSendT {
	char head;
	short device_num;
	char date[7];
	char XOR;
	char end;
}TNoRefreshSend;
enum interact_mode
{
	RECV = 1,
	SEND = 2,
};
typedef struct RecMegStructType {
	char *memory;
	size_t size;
}RecMsgStruct;
typedef struct TxRxDataT {
	char DevType;
	short DevAdr;
	short SorAdr;
	char Cmd;
	char AckType;
	short Sequence;
	char DataBlock[MAX_Buffer];
	short Crc16;
	int Block_len;
}TxRxData;
#ifdef DOOR_ALPHA
#else
#endif
