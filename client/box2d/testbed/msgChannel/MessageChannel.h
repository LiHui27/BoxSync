

#pragma once

#include "MessageQueue.h"
#include "common.h"

struct MsgPkgHead
{
	int32	iPkgLength;
	int32	iCmdID;
	uint64	ulSessionId;
};

class MessageChannel
{
public:
	MessageChannel(void);
	virtual ~MessageChannel(void);
	bool Create(int sendKey, int recvKey, unsigned int sendSize, unsigned int recvSize);

	bool RecvMsg(void *buf, unsigned int &length);
	bool SendMsg(const void *buf, unsigned int length);

	bool IsInputEmpty() const { return recvQueue_.IsQueueEmpty(); }

protected:
	MessageQueue recvQueue_;
	MessageQueue sendQueue_;
	void *sendShm_;
	void *recvShm_;
};

