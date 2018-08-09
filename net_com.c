#include<arpa/inet.h>
#include<sys/un.h.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<net/if.h>
#include<netinet/in.h>

#define MAX_MSG_LEN 1024
typedef struct RecMegStructType {
	char *memory;
	size_t size;
}RecMsgStruct;
int sck_add;
RecMsgStruct chunk_heartbeat;
int  connect_to_server()
{
	chunk_heartbeat.memory = (char *)malloc(MAX_MSG_LEN);  /* will be grown as needed by realloc above */
	chunk_heartbeat.size = 0;    /* no data at this point */

	chunk_qr.memory = (char *)malloc(MAX_MSG_LEN);  /* will be grown as needed by realloc above */
	chunk_qr.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);

	return true;
}
int GetSocketAddr(char *IPSentTo, int port)
{
	sck_add.sin_family = AF_INET;
	sck_add.sin_port = htons(port);
	sck_add.sin_addr.S_un.S_addr = inet_addr(IPSentTo);
	return 0;
}
int udp_send(int dwlen) {
	int ret;
	ret = GetSocketAddr(gSendToIP, gSendToPort);
	if (ret != 0)
		return -1;
	ret = sendto(gSck, SendDataBuffer, dwlen, 0, (sockaddr *)&sck_add, sizeof(sck_add));
	if (ret == dwlen)
	{
		return 0;
	}
	else
	{
		FileLog::GetFileLog().rotate_logger()->error("Sent to device data fail");
		return -3;
	}

}

int udp_recv(int dwlen, char *ip, int port)
{
	int dwSenderSize = sizeof(sck_add);
	char LocalHost[32];
	memset(LocalHost, 0, 32);
	int addr_len = sizeof(sck_add);
	int ret = recvfrom(gSck, RecDataBuffer, dwlen, 0, (sockaddr *)&sck_add, (int *)&addr_len);

	for (int i = 0; i < gReadTimes; i++)
	{
		if (ret == SOCKET_ERROR)
		{
			FileLog::GetFileLog().rotate_logger()->error("upd_recv try error ");
			Sleep(50);
			ret = recvfrom(gSck, RecDataBuffer, dwlen, 0, (sockaddr *)&sck_add, (int *)&addr_len);
		}
		else
		{
			break;
		}
	}
	return ret;

}
int WriteData(TSendData *dData)
{
	int len;
	int headLen = 9;
	len = dData->Block_len;

	dData->DevAdr = SWAP_WORD(dData->DevAdr);
	dData->SorAdr = SWAP_WORD(dData->SorAdr);
	dData->Sequence = SWAP_WORD(dData->Sequence);

	dData->Crc16 = GetCrc16((char *)dData, len + 9);
	memset(SendDataBuffer, 0, SEND_BUF_SIZE);
	memcpy(SendDataBuffer, (char *)dData, sizeof(TSendData));

	SendDataBuffer[headLen + len] = dData->Crc16 >> 8;
	SendDataBuffer[headLen + len + 1] = dData->Crc16 & 0xff;
	len = len + 11;
	return udp_send(len);
}

int ReadData(TReceiveData *aData)
{
	//memset(aData, 0, sizeof(aData));
	memset((void *)RecDataBuffer, 0, sizeof(RecDataBuffer));
	memset(gFromIP, 0, sizeof(gFromIP));
	int ret = udp_recv(2000, gFromIP, gFromPort);
	if (ret < 0)
	{
		FileLog::GetFileLog().rotate_logger()->error("udp_recv fail ");
		return -4;
	}
	memset(aData, 0, sizeof(TReceiveData));
	memcpy(aData, RecDataBuffer, ret);
	//receive data block length
	int i = ret - 9;

	aData->Crc16 = ((aData->DataBlock[i - 2] & 0xff) << 8) | (aData->DataBlock[i - 1] & 0xff);
	aData->DataBlock[i - 2] = '\0';
	if (!CheckCRC16((char *)aData, i - 2 + 9, aData->Crc16))
	{
		FileLog::GetFileLog().rotate_logger()->error("receive data, check CRC fail ");
		return -3;
	}
	return 0;
}
static int tcp_init()
{
	int ret,flags;
	struct sockaddr_in server_addr,client_addr;
	int len=(int)sizeof(client_addr);
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(gDoorPort);			//port set
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if((gServerSck = socket(AF_INET, SOCK_STREAM, 0))==-1){
		perror("tcp socket create");
		return -3;
	}
	int bOpt = 1;
	if((ret = setsockopt(gServerSck, SOL_SOCKET, SO_REUSEADDR, (char *)&bOpt, sizeof(bOpt)))==-1){
		perror("tcp socket setopt");
		goto svrerr;
	}
	if((ret=bind(gServerSck, (const struct sockaddr *)&server_addr, sizeof(server_addr)))==-1){
		perror("socket bind");
		goto svrerr;
	}
	if((ret=listen(gServerSck,128))==-1){
		perror("socket listen");
		goto svrerr;
	}
	if((gClientSck=accept(gServerSck,(struct sockaddr*)&client_addr,&client_addr,&len))==-1){
		perror("socket accept");
		goto svrerr;
	}
	if((flags=fcntl(gClientSck,F_GETFL))==-1){
		perror("get file attr");
		goto clierr;
	}
	flags|=O_NONBLOCK;
	if((ret=fcntl(gClientSck,F_SETFL,flags))=-1){
		perror("set noblock");
		goto clierr;
	}
	return 0;
clierr:
	close(gClientSck);
svrerr:
	close(gServerSck);
	return -3;
}
static int udp_init(){
	int sockfd;
	const int opt=1;
	int timeout=100;
	if ((sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1){
		perror("UDP socket create");
		return -1;
	}
	if(setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt))==-1){
		perror("UDP socket optset");
		return -1;
	}
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

}
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	if (realsize > MAX_MSG_LEN)
		return 0;
	RecMsgStruct *mem = (RecMsgStruct *)userp;
	mem->size = 0;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}
char* send_heartbeat_get_to_server(char *heartbeat_URL, char *QRmsg, int msgLen)
{
	CURLcode res;
    CURL *curl_heartbeat;
	printf("send heartbeat_URL:{%s}\n",heartbeat_URL);
    curl_heartbeat = curl_easy_init();
    if (curl_heartbeat) 
	{
    	/* send all data to this function  */
    	curl_easy_setopt(curl_heartbeat, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    
    	/* we pass our 'chunk' struct to the callback function */
    	curl_easy_setopt(curl_heartbeat, CURLOPT_WRITEDATA, (void *)&chunk_heartbeat);
    
    	/* some servers don't like requests that are made without a user-agent
    	field, so we provide one */
    	curl_easy_setopt(curl_heartbeat, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		curl_easy_setopt(curl_heartbeat, CURLOPT_PROXY, PROXY);
		curl_easy_setopt(curl_heartbeat, CURLOPT_URL, heartbeat_URL);
    }

	/* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
	itself */
	//curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);

	/* Perform the request, res will get the return code */
	res = curl_easy_perform(curl_heartbeat);

	if (curl_heartbeat) 
	{
		curl_easy_cleanup(curl_heartbeat);
	}
	/* Check for errors */
	if (res != CURLE_OK) 
	{
		//fprintf(stderr, "curl_easy_perform() failed: %s\n",
		//	curl_easy_strerror(res));
		FileLog::GetFileLog().rotate_logger()->info("send heartbeat fail !");
		return NULL;
	}
	else 
	{
		/*
		* Now, our chunk.memory points to a memory block that is chunk.size
		* bytes big and contains the remote file.
		*
		* Do something nice with it!
		*/
		FileLog::GetFileLog().rotate_logger()->info("send heartbeat success !");
		return chunk_heartbeat.memory;
	}
}
char* send_heartbeat_post_to_server(char *heartbeat_URL, char *QRmsg, int msgLen)
{
	CURLcode res;
	bool ret = false;

	//create curl_hearbeat
	CURL *curl_heartbeat;
	curl_heartbeat = curl_easy_init();
	if (curl_heartbeat) 
	{
		/* send all data to this function  */
		curl_easy_setopt(curl_heartbeat, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl_heartbeat, CURLOPT_WRITEDATA, (void *)&chunk_heartbeat);

		/* some servers don't like requests that are made without a user-agent
		field, so we provide one */
		curl_easy_setopt(curl_heartbeat, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	}

	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl_heartbeat, CURLOPT_POST, 1L);
	curl_easy_setopt(curl_heartbeat, CURLOPT_POSTFIELDS, QRmsg);

	curl_easy_setopt(curl_heartbeat, CURLOPT_URL, heartbeat_URL);

	curl_easy_setopt(curl_heartbeat, CURLOPT_PROXY, PROXY);
	/* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
	itself */
	curl_easy_setopt(curl_heartbeat, CURLOPT_POSTFIELDSIZE, msgLen);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "content-type: application/json");
	curl_easy_setopt(curl_heartbeat, CURLOPT_HTTPHEADER, headers);
	
	/* Perform the request, res will get the return code */
	res = curl_easy_perform(curl_heartbeat);

	if (curl_heartbeat) 
	{
		curl_easy_cleanup(curl_heartbeat);
	}
	/* Check for errors */
	if (res != CURLE_OK) 
	{
		FileLog::GetFileLog().rotate_logger()->info("send msg time fail !");
		return NULL;
	}
	else 
	{
		/*
		* Now, our chunk.memory points to a memory block that is chunk.size
		* bytes big and contains the remote file.
		*
		* Do something nice with it!
		*/
		FileLog::GetFileLog().rotate_logger()->info("send msg time success !");
		return chunk_heartbeat.memory;
	}
}
