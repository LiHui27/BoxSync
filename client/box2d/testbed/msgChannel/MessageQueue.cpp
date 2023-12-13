

#include "common.h"
#include "MessageQueue.h"

MessageQueue::MessageQueue(void)
: queueBuff_(NULL)
, queueHead_(NULL)
{
}

MessageQueue::~MessageQueue(void)
{
}

inline unsigned int MessageQueue::GetLeftSize(int headIdx, int tailIdx) const
{
	if (tailIdx >= headIdx)
		return queueHead_->size - tailIdx + headIdx - QUEUE_VACUUM_SLOT;
	return headIdx - tailIdx - QUEUE_VACUUM_SLOT;
}

inline unsigned int MessageQueue::GetSize(int headIdx, int tailIdx) const
{
	if (tailIdx >= headIdx) 
		return tailIdx - headIdx;
	return queueHead_->size - headIdx + tailIdx;
}


bool MessageQueue::RecvMsg(void *buf, unsigned int &length)
{
	if (NULL == buf)
	{
		assert__(false);
		return false;
	}
	length = 0;

	int headIdx = GetHeadIdx();  // critical data
	int tailIdx = GetTailIdx();

	if (headIdx == tailIdx) // queue is empty
	{
		length = 0;
		return false;
	}
	int size = GetSize(headIdx, tailIdx);
	if (size < sizeof(unsigned int))
	{
		assert__(false);
		return false;
	}

	unsigned char *dstBuf = (unsigned char *)&length;

	for (int i = 0; i < sizeof(unsigned int); ++i)
	{
		dstBuf[i] = queueBuff_[headIdx];
		headIdx = (headIdx + 1) % queueHead_->size;
	}
	size -= sizeof(unsigned int);
	if (length > (unsigned int)size)
	{
		assert__(false);
		return false;
	}

	if (headIdx + (int)length > queueHead_->size)
	{
		memcpy(buf, queueBuff_ + headIdx, queueHead_->size - headIdx);
		memcpy(((unsigned char*)buf) + queueHead_->size - headIdx, queueBuff_, length - (queueHead_->size - headIdx));
	}
	else
	{
		memcpy(buf, queueBuff_ + headIdx, length);
	}
	headIdx = (headIdx + length) % queueHead_->size;

	SetHeadIdx(headIdx);  // critical data;
	return true;
}

bool MessageQueue::SendMsg(const void *buf, unsigned int length)
{
	if (NULL == buf)
	{
		assert__(false);
		return false;
	}
	int headIdx = GetHeadIdx();  // critical data
	int tailIdx = GetTailIdx();

	unsigned int leftSize = GetLeftSize(headIdx, tailIdx);
	if (leftSize < length + sizeof(unsigned int))   
		return false;

	unsigned char *srcBuf = (unsigned char *)(&length);
	for (int i = 0; i < sizeof(unsigned int); ++i)
	{
		queueBuff_[tailIdx] = srcBuf[i];
		tailIdx = (tailIdx + 1) % queueHead_->size;
	}
	if (length + tailIdx > queueHead_->size)
	{
		memcpy(queueBuff_ + tailIdx, buf, queueHead_->size - tailIdx);
		memcpy(queueBuff_, ((unsigned char*)buf) + queueHead_->size - tailIdx, length - (queueHead_->size - tailIdx));
	}
	else
	{
		memcpy(queueBuff_ + tailIdx, buf, length);
	}
	tailIdx = (tailIdx + length) % queueHead_->size;

	SetTailIdx(tailIdx);   // critical data
	return true;
}

void MessageQueue::Attach(void *buf, unsigned int size, bool initHead)
{
	if (size < sizeof(QueueHead) + QUEUE_VACUUM_SLOT)
	{
		assert__(false);
		return;
	}
	queueHead_ = (QueueHead *)buf;
	queueBuff_ = (unsigned char *)(buf) + sizeof(QueueHead);
	if (initHead)
	{
		queueHead_->headIdx = 0;
		queueHead_->tailIdx = 0;
		queueHead_->size = size - sizeof(QueueHead);
	}
}


