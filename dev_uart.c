#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "dev.h"

void lock(dev_handle*dev){
	pthread_mutex_lock(&dev->mutex);
}
void unlock(dev_handle*dev){
	pthread_mutex_unlock(&dev->mutex);
}
dev_handle* dev_uart_open(char *com_name, data_callback_func cb)    //we need a private struct pointer
{
	dev_handle*dev;
	struct termios newios,oldios;
    	dev = (DEV_HANDLE*)malloc(sizeof (DEV_HANDLE));
	memset(dev,0,sizeof(DEV_HANDLE));
    	if(dev == NULL) return NULL;
	if(pthread_mutex_init(&dev->mutex,NULL)<0){
		perror("uart mutex init");
		exit(-1);
	}
    //TODO: open uart device.
#ifdef

#else

#endif
	if((dev->fd=open(com_name,O_RDWR|O_NOCTTY|O_NDELAY))<0){
		perror("uart dev open");
		exit(-1);
	}
//init device
	memset(&oldios,0,sizeof(oldios));
	memset(&newios,0,sizeof(newios));
	tcgetattr(fd,&oldios);
	newios.c_cflag=B115200|CS8|CLOCAL|CREAD;    //consider 9600.8 bit data,ignore modem,enbale read
	newios.c_iflag=IGNPAR; 						//ignore parity
	newios.c_oflag=0; 							//output mode,no need?
	newios.c_lflag=0; 							//no block
	newios.c_cflag&=~CSTOPB; 					// stop bits =1
	//newios.c_cc[VTIME]=0;  						//no timeout
	//newios.c_cc[VMIN]=1; 						//return once reading 1 chara
/*
	pNewtio->c_cc[VINTR] = 0;
	pNewtio->c_cc[VQUIT] = 0;
	pNewtio->c_cc[VERASE] = 0;
	pNewtio->c_cc[VKILL] = 0;
	pNewtio->c_cc[VEOF] = 4;
	pNewtio->c_cc[VTIME] = 5;
	pNewtio->c_cc[VMIN] = 0;
	pNewtio->c_cc[VSWTC] = 0;
	pNewtio->c_cc[VSTART] = 0;
	pNewtio->c_cc[VSTOP] = 0;
	pNewtio->c_cc[VSUSP] = 0;
	pNewtio->c_cc[VEOL] = 0;
	pNewtio->c_cc[VREPRINT] = 0;
	pNewtio->c_cc[VDISCARD] = 0;
	pNewtio->c_cc[VWERASE] = 0;
	pNewtio->c_cc[VLNEXT] = 0;
	pNewtio->c_cc[VEOL2] = 0;
*/
	tcflush(fd,TCIFLUSH); 						//reset
	tcsetattr(fd,TCSANNOW,&newios); 			//enable attr
	dev->read=uart_read;
	dev->write=uart_write;
	dev->close=uart_close;
	return dev;
}

static int uart_read(struct dev_handle *handle, U8 *data, int len)
{
    
	lock();
	read(handle->fd,data,len);
	unlock();
    return 0;
}

static int uart_write(struct dev_handle *handle, U8 *data, int len)
{
	lock();
	write(handle->write,data,len);
	unlock();
    return 0;
}

static int uart_close(struct dev_handle *handle)
{
	lock();
	close(handle->fd);
	unlock();
    	return 0;
}


