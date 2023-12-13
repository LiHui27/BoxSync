
#include "common.h"
#include "MessageChannel.h"

#include "os_shm.h"

MessageChannel::MessageChannel(void)
: sendShm_(NULL)
, recvShm_(NULL)
{
}

MessageChannel::~MessageChannel(void)
{
}


bool MessageChannel::Create(int sendKey, int recvKey, unsigned int sendSize, unsigned int recvSize)
{
	bool sendClear = false;
	bool recvClear = false;

	sendShm_ = GetShm(sendKey, sendSize, false);   
	if (NULL == sendShm_) 
	{
		sendClear = true;
		sendShm_ = GetShm(sendKey, sendSize, true);
		if (NULL == sendShm_)
			return false;
	}

	recvShm_ = GetShm(recvKey, recvSize, false);   
	if (NULL == recvShm_) 
	{
		recvClear = true;
		recvShm_ = GetShm(recvKey, recvSize, true);
		if (NULL == recvShm_)
			return false;
	}

	sendQueue_.Attach(sendShm_, sendSize, sendClear);
	recvQueue_.Attach(recvShm_, recvSize, recvClear);
	return true;
}

bool MessageChannel::RecvMsg(void *buf,  unsigned int &length)
{
	return recvQueue_.RecvMsg(buf, length);
}

bool MessageChannel::SendMsg(const void *buf, unsigned int length)
{
	return sendQueue_.SendMsg(buf, length);
}





