
#pragma once

class MessageQueue
{
public:
	MessageQueue(void);
	~MessageQueue(void);

	inline bool IsQueueEmpty() const { return queueHead_->tailIdx == queueHead_->headIdx; }
	void Attach(void *buf, unsigned int size, bool initHead);
	bool RecvMsg(void *buf, unsigned int &length);  
	bool SendMsg(const void *buf, unsigned int length);

protected:
	struct QueueHead
	{
		int headIdx;
		int tailIdx;
		int size;
	};

	inline unsigned int GetLeftSize(int headIdx, int tailIdx) const;
	inline unsigned int GetSize(int headIdx, int tailIdx) const;


	int GetHeadIdx() const { return queueHead_->headIdx; }
	int GetTailIdx() const { return queueHead_->tailIdx; }
	void SetHeadIdx(int index) { queueHead_->headIdx = index; }
	void SetTailIdx(int index) { queueHead_->tailIdx = index; }

protected:
	enum
	{
		QUEUE_VACUUM_SLOT = 8      
	};

	QueueHead *queueHead_;
	unsigned char *queueBuff_;
};







