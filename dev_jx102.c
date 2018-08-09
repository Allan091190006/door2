int  JX102R_Set_DelayValue(char delay1, char delay2, char *Ip, int port)
{
	int ret;
	gSendToPort = port;
	//	memset(gSendToIP, 0, 32);
	//	strcpy_s(gSendToIP, Ip);
	TSendData SendData;
	memset((void *)&SendData, 0, sizeof(TSendData));
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
	if (ret != 0)
	{
		return ret;
	}
	else
	{
		TReceiveData ReceiveData;
		ret = ReadData(&ReceiveData);
		if (ret != 0)
			return ret;
		else
		{
			if (ReceiveData.AckType == 0)
				return 0;
			else
			{
				FileLog::GetFileLog().rotate_logger()->error("Receive data ack type, {}\n", ReceiveData.AckType);
				return ReceiveData.AckType;
			}
		}

	}
}


int  JX102R_Open_Channel(char achannel, bool aManualClose, char *Ip, int port)
{
	int ret;
	gSendToPort = port;
	TSendData SendData;
	memset((void *)&SendData, 0, sizeof(TSendData));
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
		TReceiveData ReceiveData;
		ret = ReadData(&ReceiveData);
		if (ret != 0)
			return ret;
		else
		{
			if (ReceiveData.AckType == 0)
				return 0;
			else
			{
				FileLog::GetFileLog().rotate_logger()->error("Receive data ack type, {}\n", ReceiveData.AckType);
				return ReceiveData.AckType;
			}
		}

	}
}
int JX102R_Close_Channel(char achannel, char *Ip, int port)
{
	int ret;
	gSendToPort = port;
	//	memset(gSendToIP, 0, 32);
	//	strcpy_s(gSendToIP, Ip);

	memset((void *)&SendData, 0, sizeof(TSendData));
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
		TReceiveData ReceiveData;
		ret = ReadData(&ReceiveData);
		if (ret != 0)
			return ret;
		else
		{
			if (ReceiveData.AckType == 0)
				return 0;
			else
			{
				FileLog::GetFileLog().rotate_logger()->error("Receive data ack type, {}\n", ReceiveData.AckType);
				return ReceiveData.AckType;
			}
		}

	}
}
void chk_and_open_door(std::string str_QR_URL, std::string str_QRmsg, int msgLen)
{
	clock_t start = 0;
	clock_t ends = 0;
	start = clock();

	const char *QR_URL = str_QR_URL.c_str();
	const char *QRmsg = str_QRmsg.c_str();

	if (NULL == QRmsg)
	{
		FileLog::GetFileLog().rotate_logger()->info("QRmsg should not be null, send qr_door msg fail !");
		return;
	}

	CURLcode res;
	bool ret = false;

	CURL *curl_qr;
	curl_qr = curl_easy_init();
	if (curl_qr) 
	{
	
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
		FileLog::GetFileLog().rotate_logger()->info("curl_qr init fail! send qr_door msg fail !");
		return;
	}

	curl_easy_setopt(curl_qr, CURLOPT_POSTFIELDS, QRmsg);
	curl_easy_setopt(curl_qr, CURLOPT_URL, QR_URL);
	curl_easy_setopt(curl_qr, CURLOPT_PROXY, PROXY);
	/* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
	itself */
	curl_easy_setopt(curl_qr, CURLOPT_POSTFIELDSIZE, msgLen);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "content-type: application/json");
	curl_easy_setopt(curl_qr, CURLOPT_HTTPHEADER, headers);

	/* Perform the request, res will get the return code */
	res = curl_easy_perform(curl_qr);

	if (curl_qr)
	{
		curl_easy_cleanup(curl_qr);
		curl_qr = NULL;
	}
	if (res != CURLE_OK)
	{
		FileLog::GetFileLog().rotate_logger()->info("send qr_door msg fail !");
	}
	else
	{
		FileLog::GetFileLog().rotate_logger()->info("send qr_door msg success !");
		if (strncmp(chunk_qr.memory, "A1A2A3A41", strlen("A1A2A3A41")) == 0)
			ret = true;
		else
			ret = false;
	}

	ends = clock();
	if (ends - start > 1500)
	{
		FileLog::GetFileLog().rotate_logger()->info("send post QR_code to server take {} ms ", ends - start);
		FileLog::GetFileLog().rotate_logger()->info("send qr_door msg fail !");
		return;
		
	}

	if (ret)
	{
		FileLog::GetFileLog().rotate_logger()->info("send post QR_code to server take {} ms ", ends - start);
#ifdef DOOR_ALPHA 
		JX102R_Open_Channel(channel_number, false, gSendToIP, gSendToPort);
#else
		open_door();
#endif
		FileLog::GetFileLog().rotate_logger()->info("door opened !\n");
		PlaySound(_T("C:\\QR_CODE\\欢迎来到京东无人超市.wav"),NULL, SND_FILENAME | SND_SYNC);//单次播放
	}
	else
	{
		FileLog::GetFileLog().rotate_logger()->info("qr_code inconsistent!");
	}
}