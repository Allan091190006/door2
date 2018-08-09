#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "dev.h"

typedef struct
{
    DEV_HANDLE handle;
    int fd;
}DEV_UART_S;

static int uart_read(struct dev_handle *handle, U8 *data, int len);
static int uart_write(struct dev_handle *handle, U8 *data, int len);
static LY_STATUS uart_close(struct dev_handle *handle);
AUTO_LOCK_DECLEAR();

DEV_HANDLE* dev_uart_open(char *com_name, data_callback_func cb)    //we need a private struct pointer
{
    DEV_UART_S *dev;
	struct termios oldios,newios;
    dev = (DEV_UART_S*)malloc(sizeof (DEV_UART_S));
    if(dev == NULL) return NULL;
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
	newios.c_cflag&=~CSTOPB; 					//1 stop bit
/*
	//newios.c_cc[VTIME]=0;  						//no timeout
	//newios.c_cc[VMIN]=1; 						//return once reading 1 chara
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
	return &dev->handle;
}

static int uart_read(struct dev_handle *handle, U8 *data, int len)
{
    
	lock();
	read(,data,len);
    unlock();
    return 0;
}

static int uart_write(struct dev_handle *handle, U8 *data, int len)
{
    lock();
	write(,data,len);
    unlock();
    return 0;
}

static int uart_close(struct dev_handle *handle)
{
    lock();
	close();
    unlock();
    return 0;
}


