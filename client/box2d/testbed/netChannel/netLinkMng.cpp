#include <stdlib.h>
//#include "cocos2d.h"
//#include "MainScene.h"
//#include "snakePlayScene.h"
//#include "snakePlaySceneOnline.h"
//#include "SceneManager.h"
#include "NetLinkMng.h"

//#include "DataMng.h"
//#include <crc.h>

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/thread.h> 

#include <string>
#include <thread>

#if (defined _WIN32 || defined _WIN64) 
#include <Winsock2.h>
#define MYSLEEP(x) Sleep(x)
#define SETUP_MULTI_THREAD evthread_use_windows_threads()
#define FAMILY AF_INET
#pragma comment(lib,"ws2_32.lib")

#define CCLOG

#ifdef CCLOG
#undef CCLOG
#define CCLOG(format, ...)      printf(format, ##__VA_ARGS__)
#define CCLOG(format, ...)      printf("(%d)-<%s>: "##format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define CCLOG(format, ...)      printf(""##format, ##__VA_ARGS__)
#endif


#else
#include <arpa/inet.h>
#include <unistd.h> 
#define SETUP_MULTI_THREAD evthread_use_pthreads();
#define FAMILY AF_UNIX
#define MYSLEEP(x) usleep(x*1000)

#include <netinet/tcp.h> // for TCP_NODELAY
#include <pthread.h>

#include "netLinkMng.h"
#include "MessageChannel.h"

#define  LOG_TAG    "NN"
//#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#ifdef CCLOG
#undef CCLOG
//#define CCLOG(format, ...)      cocos2d::log(format, ##__VA_ARGS__)
#define CCLOG LOGE
//#define CCLOG(format, ...)      cocos2d::log(""##format, ##__VA_ARGS__)
#endif
#endif

static void server_msg_cb(bufferevent* bev, void* arg);
static void event_cb(bufferevent* bev, short event, void* arg);
NetLinkMng* NetLinkMng::mInstance = nullptr;


NetLinkMng* NetLinkMng::createInstance(char* ip, int port) {
	if (mInstance == 0) {
		mInstance = new NetLinkMng(ip, port);
		mInstance->init();

	}
	return mInstance;
}

NetLinkMng* NetLinkMng::getInstance() {
	return mInstance;
}

NetLinkMng::NetLinkMng(char* ip, int port)
{
	//CHECKNETOK;
	//this->mIP = ip;
	strcpy(this->mIP, ip);
	this->mPort = port;
	mEventbase = nullptr;
	mBufferevent = nullptr;
	//memset(&mHeadRecv, 0, HEADLEN);
	mDecodeStep = 0;
	mBufferevent = nullptr;
	mBufferRecv = nullptr;
	mNetState = NET_NONE;

	mLock = 0;
}

NetLinkMng::~NetLinkMng()
{
	mInstance = nullptr;
	if (mBufferevent) {
		CCLOG("mBufferevent = %p", mBufferevent);
		bufferevent_free(mBufferevent);
		mBufferevent = nullptr;
	}
	delete mThread1;
}


bool NetLinkMng::init() {
	bool ret = false;
	do
	{
		mThread = new std::thread(&NetLinkMng::run, this);//创建一个分支线程，回调到run函数里   
		//mThread->detach();
		//mThread->join(); //t.join()等待子线程myThread执行完之后，主线程才可以继续执行下去，此时主线程会释放掉执行完后的子线程资源。

		//mThread1 = new std::thread(&NetLinkMng::runSend, this);//创建一个分支线程，回调到run函数里   
		//mThread1->detach();


		ret = true;
	} while (false);

	return ret;
}


bool NetLinkMng::isNetOk() {
	return mNetState == NET_OK;
}

void NetLinkMng::run() {

	int ret;
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif
	ret = SETUP_MULTI_THREAD;
	mBufferRecv = evbuffer_new();

	event_base* base = mEventbase = event_base_new();
	evthread_make_base_notifiable(base);
	bufferevent* bev = mBufferevent = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);

	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(mPort);
	server_addr.sin_addr.s_addr = inet_addr(mIP);


	bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, this);
	ret = bufferevent_socket_connect(bev, (sockaddr*)&server_addr, sizeof(server_addr));
	if (ret < 0)
	{
		int err = EVUTIL_SOCKET_ERROR();
		CCLOG("init bufferevent_socket_connect failed. error %d (%s)\n", err, evutil_socket_error_to_string(err));
	}
	else
	{
		mNetState = NET_CONNECTING;
		ret = bufferevent_enable(bev, EV_READ | EV_PERSIST);

		//if (false)
		//{

		//	tv.tv_sec = 10; //间隔
		//	tv.tv_usec = 0;
		//	ev = evtimer_new(base, time_cb, nullptr);
		//	evtimer_add(ev, &tv);
		//	event_add(ev, &tv);
		//}

		event_base_dispatch(base);
		CCLOG("NetMng thread run finished");
	}
}


static void server_msg_cb(bufferevent* bev, void* arg) {
	if (arg)
	{
		evutil_gettimeofday(&NetLinkMng::getInstance()->mTimeval, nullptr);
		NetLinkMng::getInstance()->mNetState = NET_OK;
		NetLinkMng* obj = (NetLinkMng*)arg;
		bufferevent_lock(obj->mBufferevent);

		//evutil_gettimeofday(&(obj->stimeval), nullptr);

		obj->pack_server_msg();

		//obj->debugTime("msg_cb end");
		bufferevent_unlock(obj->mBufferevent);
	}
}

static void event_cb(bufferevent* bev, short event, void* arg) {
	if (event & BEV_EVENT_EOF) {
		CCLOG("Connection closed.\n");
		int err = EVUTIL_SOCKET_ERROR();
		CCLOG("BEV_EVENT_EOF failed. error %d (%s)\n", err, evutil_socket_error_to_string(err));
		NetLinkMng::getInstance()->mNetState = NET_CLOSEED;
	}
	else if (event & BEV_EVENT_ERROR) {
		CCLOG("Some other error.\n");
		int err = EVUTIL_SOCKET_ERROR();
		CCLOG("bufferevent_socket_connect failed. error %d (%s)\n", err, evutil_socket_error_to_string(err));

		if (true)
		{
			//Director::getInstance()->getScheduler()->performFunctionInCocosThread([] {//[&, this]
			//	std::string name = cocos2d::CCDirector::getInstance()->getRunningScene()->getName();
			//if (name == "MainScene") {
			//	MainScene* mainscene = dynamic_cast<MainScene*> (cocos2d::CCDirector::getInstance()->getRunningScene()->getChildren().at(1));
			//	NoNetEnv* nonetenv;
			//	if (nullptr == (nonetenv = dynamic_cast<NoNetEnv*> (mainscene->getChildByName("nonetenv"))))
			//	{
			//		auto node = NoNetEnv::create();
			//		node->showVisable(5);
			//		mainscene->addChild(node);
			//	}
			//	else {
			//		//nonetenv->showVisable(2);
			//	}
			//}
			//	});
		}

		int mPort = NetLinkMng::getInstance()->mPort;
		char* mIP = NetLinkMng::getInstance()->mIP;
		delete NetLinkMng::getInstance();


		MYSLEEP(1.0f * 1000);
		NetLinkMng::createInstance(mIP, mPort);
	}
	else if (event & BEV_EVENT_CONNECTED) {
		CCLOG("Client has successfully cliented.\n");
		//		return;
		NetLinkMng::getInstance()->mNetState = NET_CONNECTED;
		bufferevent_setcb(bev, server_msg_cb, 0, event_cb, arg);
		bufferevent_enable(bev, EV_READ | EV_PERSIST);


		evutil_socket_t  fd = bufferevent_getfd(bev);
		int opt = 1;
		int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt));
		if (ret == -1)
		{
			CCLOG("set TCP_NODELAY Error");
		}

		ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, sizeof(opt));
		if (ret == -1)
		{
			CCLOG("set SO_KEEPALIVE Error");
		}

		evutil_make_socket_nonblocking(fd);

		//DataMng::getInstance()->getUID();
		////范围是[a, b];
		//int b = 9999999;
		//int a = 1111111;
		//int range = b - a + 1;
		//srand((unsigned int)(time(0)));
		//int uid = rand() % range + a;
		//DataMng::getInstance()->setUID(uid);
		//CCLOG("UID = %d", uid);
		//NetMng::getInstance()->send_user_login_req(uid, "devid");
	}

}

int NetLinkMng::send_event(void* buf, int Len)
{
	int ret = 0;
	bufferevent_lock(mBufferevent);
	ret = bufferevent_write(mBufferevent, buf, Len);
	bufferevent_unlock(mBufferevent);

	return ret;
}

void NetLinkMng::pack_server_msg()
{
	bool finished = false;
	int totallen = 0;
	int targetlen = 0;

	evbuffer* input = bufferevent_get_input(mBufferevent);
	evbuffer_add_buffer(mBufferRecv, input);

	int recvLen = evbuffer_get_length(mBufferRecv);

	for (;;)
	{
		if (mDecodeStep == 0)
		{

			//if (recvLen >= HEADLEN)
			//{
			//	evbuffer_remove(mBufferRecv, &mHeadRecv, HEADLEN);
			//	mHeadRecv.val = ntohl(mHeadRecv.val);
			//	recvLen -= HEADLEN;
			//	//Log(&mHeadRecv);
			//}
			//else
			//{
			//	//CCLOG("finished or headerror, recvLen < mHeadRecv.length , %d < %d", recvLen, mHeadRecv.length);
			//	return;
			//}

			//if (recvLen >= mHeadRecv.length)
			//{
			//	mDecodeStep = 2;
			//	continue;
			//}
			mDecodeStep = 1;
			continue;
		}
		else if (mDecodeStep == 1) {
			//if (recvLen >= mHeadRecv.length)
			//{
				mDecodeStep = 2;
			//	continue;
			//}
			//else
			//{
			//	//CCLOG("recv not finished ,recvLen < mHeadRecv.length , %d < %d", recvLen, mHeadRecv.length);
			//	return;
			//}
		}
		else if (mDecodeStep == 2)
		{
			evbuffer_pullup(mBufferRecv, 24);
			
			exe_server_msg();

			evbuffer_drain(mBufferRecv, 24);
			recvLen -= 24;
			mDecodeStep = 0;
			return;
		}
#ifndef WIN32
		sched_yield();
#endif
	}
}

extern bool g_msg_send(void* buf, unsigned int& length);

int NetLinkMng::exe_server_msg()
{
	//char* msg = new char[256];
	char msg[64] = {0};
	memset(msg, 0, 64);
	evbuffer_copyout(mBufferRecv, msg, 24);
	unsigned int len = 24;
	g_msg_send(msg, len);
	return 0;
}

int NetLinkMng::setPackData(const int* crc, const std::string& data, int datalen) {
	int ret = 0;
	bufferevent_lock(mBufferevent);
	//int buflen = datalen + sizeof(int);
	//char* pack = new char[buflen + 1];
	//memset(pack, 0, buflen + 1);
	//memcpy(pack, crc, sizeof(int));
	//if (datalen > 0)
	//	memcpy(pack + sizeof(int), data.c_str(), datalen);
	//int ret = bufferevent_write(mBufferevent, pack, buflen);
	//CC_SAFE_DELETE_ARRAY(pack);
	bufferevent_unlock(mBufferevent);
	return ret;
}
