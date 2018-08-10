#ifndef _DEV_H_
struct dev_handle;
typedef int(*dev_access_func)(struct dev_handle*handle,u8*data,int len);
typedef int (*dev_close_func)(struct dev_handle*handle);
typedef int (*data_callback_func)(int index,u8*data,int len);
typedef struct dev_handlet{
	dev_access_func read,write;
	dev_close_func close;
	int fd;
	pthread_mutex_t mutex;
	void*private;
}dev_handle;
int JX102R_Set_DelayValue(char delay1, char delay2, char *Ip, int port);
int JX102R_Open_Channel(char achannel, int aManualClose, char *Ip, int port);
int JX102R_Close_Channel(char achannel, char *Ip, int port);
int play_sound(char*file);
void lock(dev_handle*dev);
void unlock(dev_handle*dev);
