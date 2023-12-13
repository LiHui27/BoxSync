#pragma once

#include <thread>
#include <mutex>
#include <event2/event.h>
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"

enum NET_STATE
{
	NET_NONE,
	NET_INIT,
	NET_CONNECTING,
	NET_CONNECTED,
	NET_OK,
	NET_CLOSEED,
};

typedef struct notifyItem
{
	char* buf;
	int buflen;
};

class NetLinkMng
{
public:
	NetLinkMng(char* ip, int port);
	~NetLinkMng();

	static NetLinkMng* getInstance();
	static NetLinkMng* createInstance(char* ip, int port);
	bool isNetOk();

	//void send_user_login_req(long long user_id, std::string dev_id);
	//void send_enter_public_matching_req(int skin_id, void* snakePlayViewOnlineLayer);
	//void send_leave_public_matching_req();
	//void send_create_room_req(create_room_req& msg);
	//void send_enter_room_req(enter_room_req& msg);
	//void send_send_room_invite_req(send_room_invite_req& msg);
	//void send_start_game_req();
	//void send_leave_room_req();
	//void send_resurgence_req();
	//void send_heartbeat_req();
	//void send_get_public_matching_room_info_req();
	//void send_use_invincible_prop_req();
	int send_event(void* buf, int Len);
	void pack_server_msg();
	int exe_server_msg();
	//void send_action_sync_req(action_sync_req& msg);
	NET_STATE mNetState;
	timeval mTimeval;
	int mPort;
	char mIP[16];


	//int pushFrame(normal_frame_notify* notifty, char*buf, int buflen, bool isPush);
	//std::vector<notifyItem> mFramenotifyVec;

	/*isPush 1->push, 0->put, 2->clear*/
	//pool<normal_frame_notify> frame_pool;
	//normal_frame_notify* give();
	//void give_back(normal_frame_notify* obj);
	//int pushFrame(normal_frame_notify*& notifty, int isPush);
	//msg_que _msg_que;
	//moodycamel::ConcurrentQueue<long> msg_queue;

	//int mFramenotifyVecLock;
	//std::mutex m_mtx_normal_frame_notify;
	//action_sync_req msg;

	int setPackData(const int* crc, const std::string& data, int datalen);
	int mLock;
	bufferevent* mBufferevent;
	timeval stimeval;
	void debugTime(char* tag);

private:
	static NetLinkMng* mInstance;
	std::thread* mThread;
	std::thread* mThread1;

	bool init();
	void run();
	void runSend();
	//void Log(CRC* head);

	event_base* mEventbase;
	//CRC mHeadRecv;
	int mDecodeStep;

	evbuffer* mBufferRecv;

	std::string token;

	void* _snakePlayViewOnlineLayer;


	std::mutex m_mtx_action_sync_req;
};

