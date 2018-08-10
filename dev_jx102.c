#include<string.h>
#include"door.h"
#include"util.h"
#define DELAY_VALUE  0x03
#define REMOTE_OPEN  0x05
#define REMOTE_CLOSE 0x06

extern gSendToPort;
int  JX102R_Set_DelayValue(char delay1, char delay2, char *Ip, int port)
{
	int ret;
	gSendToPort = port;
	//	memset(gSendToIP, 0, 32);
	//	strcpy_s(gSendToIP, Ip);
	TxRxData SendData;
	memset((void *)&SendData, 0, sizeof(TxRxData));
	SendData.DevType = 0x80;
	SendData.DevAdr = 0xFFFF;
	SendData.SorAdr = 0x0000;
	SendData.Cmd = DELAY_VALUE;
	SendData.AckType = 0xAA;
	SendData.Sequence = 0x0001;
	SendData.DataBlock[0] = delay1;
	SendData.DataBlock[1] = delay2;

	SendData.Block_len = 2;
	ret = WriteData(&SendData);
	if (ret != 0){
		return ret;
	}else{
		TxRxData ReceiveData;
		ret = ReadData(&ReceiveData);
		if (ret != 0)
			return ret;
		else	{
			if (ReceiveData.AckType == 0)
				return 0;
			else	{
				printf("%s:Receive data ack type %x\n", __func__,ReceiveData.AckType);
				return ReceiveData.AckType;
			}
		}

	}
}


int  JX102R_Open_Channel(char achannel, int aManualClose, char *Ip, int port)
{
	int ret;
	gSendToPort = port;
	TxRxData SendData;
	memset((void *)&SendData, 0, sizeof(TxRxData));
	SendData.DevType = 0x80;
	SendData.DevAdr = 0xFFFF;
	SendData.SorAdr = 0x0000;
	SendData.Cmd = REMOTE_OPEN;
	SendData.AckType = 0xAA;
	SendData.Sequence = 0x0001;
	SendData.DataBlock[0] = achannel;
	if (aManualClose)
		SendData.DataBlock[1] = 0x1;
	else
		SendData.DataBlock[1] = 0x0;

	SendData.Block_len = 2;
	ret = WriteData(&SendData);
	if (ret != 0)
	{
		return ret;
	}
	else
	{
		TxRxData ReceiveData;
		ret = ReadData(&ReceiveData);
		if (ret != 0)
			return ret;
		else{
			if (ReceiveData.AckType == 0)
				return 0;
			else	{
				printf("%s:Receive data ack type %x\n", ReceiveData.AckType);
				return ReceiveData.AckType;
			}
		}

	}
}
int JX102R_Close_Channel(char achannel, char *Ip, int port)
{
	int ret;
	TxRxData SendData;
	gSendToPort = port;
	//	memset(gSendToIP, 0, 32);
	//	strcpy_s(gSendToIP, Ip);

	memset((void *)&SendData, 0, sizeof(TxRxData));
	SendData.DevType = 0x80;
	SendData.DevAdr = 0xFFFF;
	SendData.SorAdr = 0x0000;
	SendData.Cmd = REMOTE_CLOSE;
	SendData.AckType = 0xAA;
	SendData.Sequence = 0x0001;
	SendData.DataBlock[0] = achannel;
	SendData.Block_len = 1;
	ret = WriteData(&SendData);
	if (ret != 0)
	{
		return ret;
	}
	else
	{
		TxRxData ReceiveData;
		ret = ReadData(&ReceiveData);
		if (ret != 0)
			return ret;
		else	{
			if (ReceiveData.AckType == 0)
				return 0;
			else{
				printf("%s:Receive data ack type %x\n",__func__, ReceiveData.AckType);
				return ReceiveData.AckType;
			}
		}

	}
}
