#ifndef _DEV_H_
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
struct dev_handle;
typedef int(*dev_access_func)(struct dev_handle*handle,u8*data,int len);
typedef int (*dev_close_func)(struct dev_handle*handle);
typedef int (*data_callback_func)(int index,u8*data,int len);
typedef struct dev_handle{
	dev_access_func read,write;
	dev_close_func close;
}DEV_HANDLE;
