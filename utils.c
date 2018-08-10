#include<stdio.h>
#include<time.h>
#include<sys/time.h>
#include"cJSON.h"
#define SUCCESS_JASON_CODE  "0000"
#define HEARTBEAT_CMD_MAX  8
#define HBCMD_SENDTIME_STR  "sendTime"
#define TERMINAL_TYPE            "01"
short CRCTable16[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

short GetCrc16(char *pData, int nLength)
{
	short fcs = 0;
	for (int i = 0; i < nLength; i++){
		fcs = ((fcs & 0xff) << 8) ^ CRCTable16[(((fcs >> 8) & 0xff) ^ ((*pData)) & 0xFF)];
		pData++;
	}
	return fcs;
}

int CheckCRC16(char *pData, int nLength, short CrcValue)
{
	short fcs = 0;
	for (int i = 0; i < nLength; i++){
		fcs = ((fcs & 0xff) << 8) ^ CRCTable16[(((fcs >> 8) & 0xff) ^ (*pData)) & 0xFF];
		pData++;
	}
#if defined	DEBUG_MODE
	cout << hex << "check CRC " << fcs << " " << CrcValue << endl;
#endif
	return fcs == CrcValue;
}
void epoll_ADD(int epfd,int fd){
	struct epoll_event ev;
	ev.data.fd=fd;
	ev.events=EPOLLIN|EPOLLET;
	if(epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev)==-1){
		perror("epoll_ADD");
		exit(-1);
	}
}
void epoll_DEL(int epfd,int fd){
	struct epoll_event ev;
	ev.data.fd=fd;
	if(epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&ev)==-1)
	perror("epoll_DEL");

}
void time_creator(int*fd ,long interval){
	struct itimerspec time_new;
	struct timespec now;
	long long nosec;
	int sec,us;
	if((*fd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK))<0){
		perror("timer create");
		exit(-1);
	}
	clock_gettime(CLOCK_MONOTONIC,&now);
	sec=interval/1000000;
	us=interval%1000000;
	nosec=((long long)now.tv_sec+sec)*1000*1000*1000+now.tv_nsec+us*1000;
	time_new->it_value.tv_sec=nosec/(1000*1000*1000);
	time_new->it_value.tv_nsec=nosec%(1000*1000*1000);
	time_new->it_interval.tv_sec=sec;
	time_new->it_interval.tv_nsec=us*1000;
	timerfd_settime(*fd,TFD_TIMER_ABSTIME,&time_new,0);
}
int add_timer(int epfd,int timerfd){
	int ret;
	struct epoll_event ev;
	ev.data.fd=timerfd;
	ev.events=EPOLLIN|EPOLLET|EPOLLOUT;
	if((ret=epoll_ctl(epfd,EPOLL_CTL_ADD,timerfd,&ev))<0)
		return -1;
	return 0;
}
int del_timer(int epfd,int timerfd){
	int ret;
	struct epoll_event ev;
	ev.data.fd=timerfd;
	if((ret=epoll_ctl(epfd,EPOLL_CTL_DEL,timerfd,&ev))!=0)
		return -1;
	return 0;
}
long get_systime(void){
	struct timeval tv;
	long value=0;
	memset(&tv,0,sizeof(tv));
	gettimeofday(&tv,NULL);
	value=tv.tv_sec*1000+tv.tv_usec/1000;
	return value;
}
char* serialize_json_sendtime(const char *storeid, const char *sourceid, const char *reqid){
	cJSON *root, *termtime;
	char *json = NULL;
	struct timeval tv;
	static char tv_str[64];
	long long time_stamp;
	memset(tv_str, 0, 64);
	root = cJSON_CreateObject();
	termtime = cJSON_CreateObject();
	if (root == NULL || termtime == NULL){
		goto err;
	}

	cJSON_AddStringToObject(root, "reqId", reqid);
	cJSON_AddStringToObject(root, "terminalType", TERMINAL_TYPE);

	time_stamp = get_systime();
	printf("%s:time [%llu]\n",time_stamp);
	snprintf(tv_str, 63, "%llu", time_stamp);
	cJSON_AddStringToObject(termtime, sourceid, tv_str);
	cJSON_AddItemToObject(root, "terminalTime", termtime);

	json = cJSON_Print(root);
	if(root)
		cJSON_Delete(root);
	return json;
err:
	if (root){
		cJSON_Delete(root);
	}
	return NULL;
}
int WriteData(TxRxData*dData)
{
	int len;
	int headLen = 9;
	len = dData->Block_len;

	dData->DevAdr = SWAP_WORD(dData->DevAdr);
	dData->SorAdr = SWAP_WORD(dData->SorAdr);
	dData->Sequence = SWAP_WORD(dData->Sequence);

	dData->Crc16 = GetCrc16((char *)dData, len + 9);
	memset(SendDataBuffer, 0, SEND_BUF_SIZE);
	memcpy(SendDataBuffer, (char *)dData, sizeof(TxRxData));

	SendDataBuffer[headLen + len] = dData->Crc16 >> 8;
	SendDataBuffer[headLen + len + 1] = dData->Crc16 & 0xff;
	len = len + 11;
	return udp_send(len);
}

int ReadData(TxRxData*aData)
{
	int ret;
	memset((void *)RecDataBuffer, 0, sizeof(RecDataBuffer));
	memset(gFromIP, 0, sizeof(gFromIP));
	ret = udp_recv(2000, gFromIP, gFromPort);
	if (ret < 0){
		printf("%s:udp_recv fail ",__func__);
		return -4;
	}
	memset(aData, 0, sizeof(TxRxData));
	memcpy(aData, RecDataBuffer, ret);
	//receive data block length
	int i = ret - 9;
	aData->Crc16 = ((aData->DataBlock[i - 2] & 0xff) << 8) | (aData->DataBlock[i - 1] & 0xff);
	aData->DataBlock[i - 2] = '\0';
	if (!CheckCRC16((char *)aData, i - 2 + 9, aData->Crc16))
	{
		printf("%s:receive data, check CRC fail ",__func__);
		return -3;
	}
	return 0;
}
char * serialize_json_QR(const char * token,const char*gateid){
	cJSON*root=NULL;
	char*json=NULL;
	long long time_stamp=0;
	static char tv_str[64];
	memset(tv_str,0,64);
	if((root=cJSON_CreateObject())==NULL){
		printf("%s:obj create\n",__func__);
		return NULL;
	}
	cJSON_AddStringToObject(root,"token",token);
	cJSON_AddStringToObject(root,"gateid",gateid);
	time_stamp=get_systime();
	snprintf(tv_str,63,"%llu",time_stamp);
	cJSON_AddStringToObject(root,"timestamp",tv_str);
	json=cJSON_Print(root);
	if(root)
		cJSON_Delete(root);
	return json;
}
/*
@clock ? process time ,what if this prog was schedule out?
*/
void chk_and_open_door(void*arg)
{
	clock_t start = 0;
	clock_t end = 0;
	CURL *curl_qr;
	CURLcode res;
	/*
	char*str_QRmsg=malloc(strlen((char*)arg));
	char*str_QR_URL=(char*)malloc(sizeof(QR_IP_HEAD));
	*/
	int ret = -1;
	struct curl_slist *headers = NULL;
	int msgLen=sizeof(strlen(char*)arg);
	const char *QR_URL = QR_IP_HEAD;
	const char *QRmsg =(char*)arg;
	start = clock();
	if (NULL == QRmsg){
		printf("%s:QRmsg should not be null, send qr_door msg fail !",__func__);
		return;
	}
	curl_qr = curl_easy_init();
	if (curl_qr){
	
		/* send all data to this function  */
		curl_easy_setopt(curl_qr, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	
		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl_qr, CURLOPT_WRITEDATA, (void *)&chunk_qr);
	
		/* some servers don't like requests that are made without a user-agent
		field, so we provide one */
		curl_easy_setopt(curl_qr, CURLOPT_USERAGENT, "libcurl-agent/1.0");	
	}
	else
	{
		printf("%s:curl_qr init fail! send qr_door msg fail !",__func__);
		return;
	}

	curl_easy_setopt(curl_qr, CURLOPT_POSTFIELDS, QRmsg);
	curl_easy_setopt(curl_qr, CURLOPT_URL, QR_URL);
	curl_easy_setopt(curl_qr, CURLOPT_PROXY, PROXY);
	/* if we don't provide POSTFIELDSIZE, libcurl will strlen() by itself */
	curl_easy_setopt(curl_qr, CURLOPT_POSTFIELDSIZE, msgLen);
	headers = curl_slist_append(headers, "content-type: application/json");
	curl_easy_setopt(curl_qr, CURLOPT_HTTPHEADER, headers);
	/* Perform the request, res will get the return code */
	res = curl_easy_perform(curl_qr);

	if (curl_qr){
		curl_easy_cleanup(curl_qr);
		curl_qr = NULL;
	}
	if (res != CURLE_OK){
		printf("%s:send qr_door msg fail !"__func__);
	}else{
		printf("%s:send qr_door msg success !",__func__);
		if (strncmp(chunk_qr.memory, "A1A2A3A4", strlen("A1A2A3A4")) == 0)
			ret = 1;
		else
			ret = 0;
	}
	end = clock();
	if (end - start > 1500)
	{
		printf("%s:send post QR_code to server take %ld ms ",__func__, ends - start);
		printf("%s:send qr_door msg fail !",__func__);
		return;
	}
	if (ret){
		printf("%s:send post QR_code to server take %ld ms ",__func__, ends - start);
#ifdef DOOR_ALPHA 
		JX102R_Open_Channel(channel_number, 0, gSendToIP, gSendToPort);
#else
		open_door();
#endif
		printf("%s:door opened !\n",__func__);
		play_sound("C:\\QR_CODE\\欢迎来到京东无人超市.wav");//单次播放
	}else{
		printf("%s:qr_code inconsistent!",__func__);
	}
}