#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<stdio.h>
#include<errno.h>
#include<pthread.h>
#include<curl/curl.h>
#include"door.h"
#include "cJSON.h"
#define EPOLL_MAXFD 5
#define CONFIG_FILE 		"ALPHA_QR_CONFIG.INI"
#define QR_IP_HEAD_STR 		"QR_IP_HEAD"
#define QR_SCAN_STR	  		"QR_SCAN"
#define HARTBT_URL_STR 		"heartbeat_URL"
#define COMM_NUMBER_STR 	"comm_number"
#define GSENDTOIP_STR		"gSendToIP"
#define GFROMIP_STR 		"gFromIP"
#define GSENDTOPORT_SRT		"gSendToPort"
#define PROXY_STR			"PROXY"
#define STORE_ID_STR		"STORE_ID"
#define MAC_ADDR_STR		"MAC_ADDR"
#define CHANNEL_NUMBER_STR 	"channel_number"
#define CBR_NUMBER_STR		"CBR_NUMBER"
#define HARTBT_SND_URL_STR 	"heartbeat_send_URL"
#define REC_BUF_SIZE 		2048
#define SEND_BUF_SIZE 		2048
#define MAX_BUF 			1032

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
int CBR_NUMBER;
int channel_number;
int comm_number;
int gSendToPort;
char QR_IP_HEAD[256];
char QR_SCAN[256];
char PROXY[32];
char gSendToIP[32];
char gFromIP[32];
char STORE_ID[20];
char MAC_ADDR[20];
char heartbeat_URL[256];
char heartbeat_send_URL[256];

int gServerSck;
int gClientSck;

int bOpenDoor=0;
int gDoorPort=20001;
char hb_head_buf[]={0x31};
char RecDataBuffer[REC_BUF_SIZE];
char SendDataBuffer[SEND_BUF_SIZE];
static pthread_mutex_t global_mutex=PTHREAD_MUTEX_INITIALIZER;
short channel=0xFD76;//belta 0xE876 //'yinlian': FD76
char  text[] = { 0xE0, 0x07, 0xBF, 0xAA,0xC3, 0xC5, 0xB2, 0xE2, 0xCA, 0xD4,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
char voice[] = { 0xBF, 0xAA, 0xC3, 0xC5, 0xB2, 0xE2, 0xCA, 0xD4, 0x00, 0x00,
			     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00 };
char get_xor(char* buf, unsigned int buf_len)
{
	if (buf_len > 0 && buf)
	{
		char cXor = 0x00;
		for (int cnt = 0; cnt < buf_len; cnt++)
			cXor ^= buf[cnt];
		return(cXor & 0xff);
	}
	return 0x00;
}
int check_xor(char* buf, unsigned int buf_len, char orin_xor)
{
	char des_xor = get_xor(buf, buf_len);
	return(des_xor == orin_xor);
}
void open_door(){
	pthread_mutex_lock(&global_mutex);
	bOpenDoor=1;
	pthread_mutex_unlock(&global_mutex);
	return ;
}
/*
@response_jparse: parse response from server,which is ....??

*/
int response_jparse(char*json){
	cJSON*root=cJSON_Parse(json);
	cJSON*code,*msg,*cmd,*key, *val, *reqId;
	size_t keylen;
	int num, i;
	int retval = -1;
	
	if (root == NULL) {
		printf("%s:Invalid json text\n",__func__);
		retval = -1;
		return retval;
	}

	code = cJSON_GetObjectItem(root, "code");
	if (code == NULL || strcmp(code->valuestring, SUCCESS_JASON_CODE) != 0){
		printf("%s:Invalid code\n",__func__);
		return retval;
	}

	if((msg = cJSON_GetObjectItem(root, "msg"))==NULL)	{
		printf("%s:Null msg\n",__func__);
		return 0;
	}

	num=cJSON_GetArraySize(msg);
	if (num > HEARTBEAT_CMD_MAX)
		num = HEARTBEAT_CMD_MAX;

	for (i = 0; i < num; i++) 
	{
		printf("%s:%d", __func__,i,);
		if((cmd = cJSON_GetArrayItem(msg, i))==NULL){
			printf("%s:msg[%d] is empty",__func__,i);
			return 0;
		}

		if((key=cJSON_GetObjectItem(cmd, "command"))== NULL){
 		 	printf("%s:no cmd in msg[%d]\n"__func__,i);
			return 0;
		}else{
			if(strncmp(key->valuestring,"open",strlen("open"))==0){
				JX102R_Open_Channel(channel_number, 1, gSendToIP, gSendToPort);
				printf("%s:remote open\n",__func__);
			}else if (strncmp(key->valuestring, "close", strlen("close")) == 0)	{
				JX102R_Close_Channel(channel_number,  gSendToIP, gSendToPort);
				printf("%s:remote close\n",__func__);
			}else if(strlen(HBCMD_SENDTIME_STR) == strlen(key->valuestring)
			&& strncmp(key->valuestring, HBCMD_SENDTIME_STR, keylen)==0){


			}else{
				printf("%s:error cmd :%s\n",__func__,key->valuestring);
			}
		}
	if((reqId=cJSON_GetObjectItem(cmd, "reqId"))==NULL){
			printf("%s:no reqId found\n",__func__);
			return 0;
	}
				char *time_msg = serialize_json_sendtime(STORE_ID, mac_addr, reqId->valuestring);
				FileLog::GetFileLog().rotate_logger()->info("time_msg: {}", time_msg);
				send_heartbeat_post_to_server(heartbeat_send_URL, time_msg, strlen(time_msg));
			}
		}
		else {
			FileLog::GetFileLog().rotate_logger()->error("NULL HBCMD_SENDTIME_STR \n");
			return 0;
		}
	}
	return retval;
}
void SendHeartbeat(char*heartbeat_URL){
	char*response=NULL;
	response=send_heartbeat_get_to_server();	
	if(response!=NULL)
		response_jparse(response);
}
void task_process(){
	clock_t start,end;
	int m_read,m_write,qr_scan_len;
	int epoll_fd=0;
	int nfds,idx;
	struct epoll_event events[EPOLL_MAXFD];
	if(-1==(epoll_fd=epoll_create(EPOLL_MAXFD))){
		perror("epoll_create");
		exit(-1);
	}
	epoll_ADD(epoll_fd,targetfd);
	strncpy(heartbeat_URL+strlen(heartbeat_URL),MAC_ADDR,strlen(MAC_ADDR));
	while(1){
		if((nfds=epoll_wait(epoll_fd,events,MAXFD,-1))<0){
			if(errno!=EINTR)
			perror("epoll_wait");
		}else if(nfds>0){
			for(idx=0;idx<nfds;idx++)
				if(events[idx].data.fd==targetfd){
					start=clock();
					usleep(70000);
					//TODO: Readcommblock
					end=clock();
					if(end-start>250)
						m_read=m_write=0;
					else{
						int qr_scan_len=m_write-m_read;
						if(qr_scan_len>0){
							char*token=malloc(qr_scan_len+1);	
							memcpy(token,,qr_scan_len);  //where is source allocated buf start addr + m_read
							token[qr_scan_len]='\0';
							m_read=m_write=0;
			#ifdef DOOR_ALPHA
							printf("%s:received QR code=[%s]len=[%ld]\n",token,strlen(token));
							char*QRmsg=serialize_json_QR(token,"");
							if(NULL=QRmsg)
								continue;
			#else
							//TODO remove redundancy characeter
							char*QRmsg=serialize_json_QR(token,"");
							if(NULL=QRmsg)
								continue;
			#endif
						pthread_create(&,NULL,chk_and_open_door,(void*)QRmsg);	
						}
					
					
					}

				}
		
		
		
		
		}if(events[idx].data.fd=){
			SendHeartbeat(heartbeat_URL);
		}
		
	
	}
		}

}
static int init_server_configure(void){
	cJSON*root=NULL,*tmp=NULL;
	int conf_fd,ret=-1;
	off_t size;
	char *buf=NULL;
	struct stat conf;
	memset(&conf,0,sizeof(conf));
	if((ret=stat(CONFIG_FILE,&conf))==-1){
		perror("get config file stat");
		exit(-1);
	}
	size=conf.st_size&~0xFFF+0x1000;
	if((conf_fd=open(CONFIG_FILE,O_RDONLY))<0){
		perror("Config file open");
		exit(-1);	
	}
	if((buf=malloc(size))==NULL){
		perror("malloc buffer for config file");
		exit(-1);
	}
		
	memset(buf,0,sizeof(buf));
	while(read(conf_fd,buf,conf.st_size)<0&&errno==EINTR);
	if((root=cJSON_Parse(buf))==NULL){
		printf("%s:",__func__);
		exit(-1);
	}
	if((tmp=cJSON_GetObjectItem(root,QR_IP_HEAD_STR))==NULL){
		printf("%s:get %s err\n",__func__,QR_IP_HEAD_STR);
		exit(-1);
	}
	strcpy(QR_IP_HEAD,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,QR_SCAN_STR))==NULL){
		printf("%s:get %s err\n",__func__,QR_SCAN_STR);
		exit(-1);
	}
	strcpy(QR_SCAN,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,HARTBT_URL_STR))==NULL){
		printf("%s:get %s err\n",__func__,HARTBT_URL_STR);
		exit(-1);
	}
	strcpy(heartbeat_URL,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,COMM_NUMBER_STR))==NULL){
		printf("%s:get %s err\n",__func__,COMM_NUMBER_STR);
		exit(-1);
	}
	comm_number=tmp->valueint;
	if((tmp=cJSON_GetObjectItem(root,GSENDTOIP_STR))==NULL){
		printf("%s:get %s err\n",__func__,GSENDTOIP_STR);
		exit(-1);
	}
	strcpy(gSendToIP,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,GFROMIP_STR))==NULL){
		printf("%s:get %s err\n",__func__,GFROMIP_STR);
		exit(-1);
	}
	strcpy(gFromIP,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,GSENDTOPORT_SRT))==NULL){
		printf("%s:get %s err\n",__func__,GSENDTOPORT_SRT);
		exit(-1);
	}
	gSendToPort=tmp->valueint;
	if((tmp=cJSON_GetObjectItem(root,PROXY_STR))==NULL){
		printf("%s:get %s err\n",__func__,PROXY_STR);
		exit(-1);
	}
	strcpy(PROXY,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,STORE_ID_STR))==NULL){
		printf("%s:get %s err\n",__func__,STORE_ID_STR);
		exit(-1);
	}
	strcpy(STORE_ID,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,MAC_ADDR_STR))==NULL){
		printf("%s:get %s err\n",__func__,MAC_ADDR_STR);
		exit(-1);
	}
	strcpy(MAC_ADDR,tmp->valuestring);
	if((tmp=cJSON_GetObjectItem(root,CHANNEL_NUMBER_STR))==NULL){
		printf("%s:get %s err\n",__func__,CHANNEL_NUMBER_STR);
		exit(-1);
	}
	channel_number=tmp->valueint;
	if((tmp=cJSON_GetObjectItem(root,CBR_NUMBER_STR))==NULL){
		printf("%s:get %s err\n",__func__,CBR_NUMBER_STR);
		exit(-1);
	}
	CBR_NUMBER=tmp->valueint;
	if((tmp=cJSON_GetObjectItem(root,HARTBT_SND_URL_STR))==NULL){
		printf("%s:get %s err\n",__func__,HARTBT_SND_URL_STR);
		exit(-1);
	}
	strcpy(heartbeat_send_URL,tmp->valuestring);
	cJSON_Delete(root);
	free(buf);
	close(conf_fd);
}
static int connect_com(int port){
	char com_port[10]={0};
#ifdef USB_COM
	sprintf(com_port,"ttyUSB%d",port);
#else
	sprintf(com_port,"ttyS%d",port);
#endif
	dev_uart_open(com_port);
}
int server_interact(char* buf, unsigned int buf_len, interact_mode inter_mode)
{
	int nTotalByte = 0;
	if (0 == gClientSck){
		if (0 == tcp_init()){
			return false;
		}
	}

	int nResult = -2;
	do
	{
		if (RECV == inter_mode)
			nResult = recv(gClientSck, buf + nTotalByte, buf_len - nTotalByte, 0);
		else
			nResult = send(gClientSck, buf + nTotalByte, buf_len - nTotalByte, 0);

		if (nResult > 0)
			nTotalByte += nResult;
		else
			break;
	} while (nTotalByte < buf_len);
// err handle
		closesocket(gClientSck);
		closesocket(gServerSck);
		gClientSck = -1;
		gServerSck = -1;
		if (RECV == inter_mode) //?
			FileLog::GetFileLog().rotate_logger()->error("recv door message error: {}", nError);
		else
			FileLog::GetFileLog().rotate_logger()->error("send door message error: {}", nError);

		return false;
	}

}
int SendNoRefreshData()
{
	TNoRefreshSend NoRefreshSendData;
	memset((void *)&NoRefreshSendData, 0, sizeof(NoRefreshSendData));

	NoRefreshSendData.head = 0x31;
	NoRefreshSendData.device_num = channel;

	time_t now = time(0);
	struct tm* format_time = localtime(&now);
	int year = (format_time->tm_year + 1900) % 100;
	NoRefreshSendData.date[0] = year & 0xff;
	NoRefreshSendData.date[1] = format_time->tm_mon & 0xff;
	NoRefreshSendData.date[2] = format_time->tm_mday & 0xff;
	NoRefreshSendData.date[3] = format_time->tm_hour & 0xff;
	NoRefreshSendData.date[4] = format_time->tm_min & 0xff;
	NoRefreshSendData.date[5] = format_time->tm_sec & 0xff;
	NoRefreshSendData.date[6] = (format_time->tm_wday + 1) & 0xff;

	NoRefreshSendData.XOR = get_xor((char*)(&NoRefreshSendData), sizeof(NoRefreshSendData) - 2);
	NoRefreshSendData.end = 0xff;

	memset(SendDataBuffer, 0, SEND_BUF_SIZE);
	memcpy(SendDataBuffer, (char*)(&NoRefreshSendData), sizeof(NoRefreshSendData));

	return server_interact((char*)(&SendDataBuffer), 12, interact_mode(SEND));
}
int send_refresh_data(){
	TRefreshSend RefreshSendData;
	memset((void*)&RefreshSendData,0,sizeof(RefreshSendData));
	RefreshSendData.head=0x32;
	RefreshSendData.device_num=channel;
	time_t now=time(0);
	struct tm*format_time=localtime(&now);
	int year=(format_time->tm_year+1900)%100;
	RefreshSendData.date[0] = year & 0xff;
	RefreshSendData.date[1] = format_time->tm_mon & 0xff;
	RefreshSendData.date[2] = format_time->tm_mday & 0xff;
	RefreshSendData.date[3] = format_time->tm_hour & 0xff;
	RefreshSendData.date[4] = format_time->tm_min & 0xff;
	RefreshSendData.date[5] = format_time->tm_sec & 0xff;
	RefreshSendData.date[6] = (format_time->tm_wday + 1) & 0xff;

	//activate struct
	RefreshSendData.MacActive[0] = 0x35;
	RefreshSendData.MacActive[1] = 0x01;
	RefreshSendData.MacActive[2] = 0x01;
	RefreshSendData.MacActive[3] = 0x01;
	RefreshSendData.MacActive[4] = 0x01;

	for (int i = 0; i < 6; ++i)
		memcpy(RefreshSendData.MacActive + 5 + i * 23, text, 23);

	memcpy(RefreshSendData.MacActive + 5 + 6 * 23, voice, 41);

	RefreshSendData.XOR = get_xor((char*)(&RefreshSendData), sizeof(RefreshSendData) - 2);
	RefreshSendData.end = 0xff;

	memset(SendDataBuffer, 0, SEND_BUF_SIZE);
	memcpy(SendDataBuffer, (char*)(&RefreshSendData), 268);

	int len = sizeof(RefreshSendData);
	return server_interact(SendDataBuffer, sizeof(RefreshSendData), interact_mode(SEND));
}
void ReadHeartBeatData(int buf_len){
	if(0==memcmp(RecDataBuffer,hb_head_buf,1)&&check_xor(RecDataBuffer,buf_len-2,RecDataBuffer[buf_len - 2])){
		printf("%s:get heart beat code from qr_door!",__func__);
		SendNoRefreshData();
	}
}
void *tcp_process(void*arg){
	clock_t start,end;
	while(1){
		if(bOpenDoor){
			start=clock();
			usleep(100000);
			send_refresh_data();
			end=clock();
			printf("send open door time %d\n",end-start);
			pthread_mutex_lock(&global_mutex);
			bOpenDoor=0;
			pthread_mutex_unlock(&global_mutex);

		}
		memset(RecDataBuffer,0,REC_BUF_SIZE);
		server_interact(RecDataBuffer,5,RECV);
		ReadHeartBeatData(5);
		usleep(50000);
	}
}
int main(int argc, const char *argv[])
{
	int ret;
	pthread_t tcp_pthread;
	init_server_configure();
	connect_to_server();
	connect_com(comm_number);
#ifdef DOOR_ALPHA
	udp_init();
#else
	pthread_create(&tcp_pthread,NULL,tcp_process,NULL);
#endif
	task_process();
	return 0;
}
