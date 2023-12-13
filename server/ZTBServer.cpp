#if 0
/*
  This exmple program provides a trivial server program that listens for TCP
  connections on port 9995.  When they arrive, it writes a short message to
  each client connection, and closes each connection once it is flushed.

  Where possible, it exits cleanly in response to a SIGINT (ctrl-c).
*/
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <map>

static const char MESSAGE[] = "Hello, World!\n";

static const int PORT = 9995;

static void listener_cb(struct evconnlistener *, evutil_socket_t,
	struct sockaddr *, int socklen, void *);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);

typedef std::map<evutil_socket_t, struct bufferevent *> CLIENT_MAP_DEF;
CLIENT_MAP_DEF cliMap;

int
main(int argc, char **argv)
{
	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;

	struct sockaddr_in sin;
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

	if (!signal_event || event_add(signal_event, NULL) < 0) {
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	printf("done\n");
	return 0;
}

static void
listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
	struct sockaddr *sa, int socklen, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct bufferevent *bev;
	printf("listener_cb 1\n");
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_disable(bev, EV_READ);
	//printf("listener_cb send %s \n", MESSAGE);
	//bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
	cliMap[fd] = bev;
}

static void
conn_writecb(struct bufferevent *bev, void *user_data)
{
	struct evbuffer *output = bufferevent_get_output(bev);
	printf("conn_writecb 1\n");
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");
		bufferevent_free(bev);
	}
}

static void
conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
	printf("conn_writecb %x\n", events);
	if (events & BEV_EVENT_EOF) {
		printf("Connection closed.\n");
	}
	else if (events & BEV_EVENT_ERROR) {
		printf("Got an error on the connection: %s\n",
			strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	bufferevent_free(bev);
}
#include <ctime>
static void
signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 2, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");
	CLIENT_MAP_DEF::iterator it;
	char sendBuff[128] = {0};
	if (cliMap.find(sig) != cliMap.end())
	{
		cliMap.find(sig)->second;
		for (CLIENT_MAP_DEF::iterator it = cliMap.begin(); it != cliMap.end(); it++)
		{
			//user_data->
			sprintf(sendBuff, "fd-%d,t-%d,hello", it->first, time(nullptr));
			bufferevent_write(it->second, sendBuff, strlen(sendBuff));
		}

	}

	event_base_loopexit(base, &delay);
}
#endif //if 1

#if 0
/**
You need libevent2 to compile this piece of code
Please see: http://libevent.org/
Cmd to compile this piece of code:
You need libevent2 to compile this piece of code
Please see: http://libevent.org/
Cmd to compile this piece of code:
gcc client.c -levent -o client.out
gcc server.c -levent -o server.out
**/
#include<stdio.h>  
#include<string.h>  
#include<errno.h>  

#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#include<unistd.h>  
#endif
#include<event.h>

void accept_cb(int fd, short events, void* arg);
void socket_read_cb(int fd, short events, void* arg);

int tcp_server_init(int port, int listen_num);

int main(int argc, char const *argv[])
{
	/* code */
	int listener = tcp_server_init(9999, 10);
	if (listener == -1)
	{
		perror("tcp_server_init error");
		return -1;
	}

	struct event_base* base = event_base_new();

	// 监听客户端请求链接事件
	struct event* ev_listen = event_new(base, listener, EV_READ | EV_PERSIST, accept_cb, base);

	event_add(ev_listen, NULL);

	event_base_dispatch(base); //调度

	return 0;
}

void accept_cb(int fd, short events, void* arg)
{
	evutil_socket_t sockfd;

	struct sockaddr_in client;
	socklen_t len = sizeof(client);

	sockfd = accept(fd, (struct sockaddr*)&client, &len);
	evutil_make_socket_nonblocking(sockfd);

	printf("accept a client %d\n", sockfd);

	struct event_base* base = (struct event_base*)arg;

	//动态创建一个event结构体，并将其作为回调参数传递给
	struct event* ev = event_new(NULL, -1, 0, NULL, NULL);
	event_assign(ev, base, sockfd, EV_READ | EV_PERSIST, socket_read_cb, (void*)ev);

	event_add(ev, NULL);
}


void socket_read_cb(int fd, short events, void* arg)
{
	char msg[4096];
	struct event* ev = (struct event*)arg;
	int len = read(fd, msg, sizeof(msg) - 1);

	if (len <= 0)
	{
		printf("some error happen when read\n");
		event_free(ev);
		close(fd);
		return;
	}

	msg[len] = '\0';
	printf("recv the client msg : %s\n", msg);

	char reply_msg[4096] = "I have received the msg: ";
	strcat(reply_msg + strlen(reply_msg), msg);

	write(fd, reply_msg, strlen(reply_msg));
}

typedef struct sockaddr SA;
int tcp_server_init(int port, int listen_num)
{
	int errno_save;
	evutil_socket_t listener;

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1)
		return -1;

	//允许多次绑定同一个地址。要用在socket和bind之间  
	evutil_make_listen_socket_reuseable(listener);

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(port);

	if (bind(listener, (SA*)&sin, sizeof(sin)) < 0)
		goto error;

	if (listen(listener, listen_num) < 0)
		goto error;


	//跨平台统一接口，将套接字设置为非阻塞状态  
	evutil_make_socket_nonblocking(listener);

	return listener;

error:
	errno_save = errno;
	evutil_closesocket(listener);
	errno = errno_save;

	return -1;
}
#endif


#if 1  //代码3
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <map>
#include<cassert>
#pragma comment (lib,"ws2_32.lib")
#include<ws2tcpip.h>
#define LISTEN_PORT 9867
#define LIATEN_BACKLOG 32
using namespace std;


/*********************************************************************************
*                                      函数声明
**********************************************************************************/
//accept回掉函数
void do_accept_cb(evutil_socket_t listener, short event, void *arg);
//read 回调函数
void read_cb(struct bufferevent *bev, void *arg);
//error回调函数
void error_cb(struct bufferevent *bev, short event, void *arg);
//write 回调函数
void write_cb(struct bufferevent *bev, void *arg);
/*********************************************************************************
*                                      函数体
**********************************************************************************/
typedef std::map<struct bufferevent *, evutil_socket_t>  NET_CLIENT_MAP;
NET_CLIENT_MAP g_cliMap;
static int addClient(evutil_socket_t fd, struct bufferevent *evb)
{
	g_cliMap[evb] = fd;
	return 0;
}
static int removeClient(struct bufferevent *evb)
{
	if (g_cliMap.find(evb) != g_cliMap.end())
	{
		g_cliMap.erase(evb);
		return 0;
	}
	return 1;
}
BOOL findClient(struct bufferevent *evb)
{
	NET_CLIENT_MAP::iterator it = g_cliMap.find(evb);
	if (it != g_cliMap.end())
	{
		return TRUE;
	}
	return FALSE;
}

evutil_socket_t getClientFD(struct bufferevent *evb)
{
	NET_CLIENT_MAP::iterator it = g_cliMap.find(evb);
	if (it != g_cliMap.end())
	{
		return it->second;
	}
	return 0;
}

void BoardMessage(char* Msg, int Len)
{
	for (NET_CLIENT_MAP::iterator it = g_cliMap.begin(); it != g_cliMap.end(); it++)
	{
		bufferevent_write(it->first, Msg, Len);
		printf("send:%s,to:%d\n", Msg, it->second);
	}
}

//accept回掉函数
void do_accept_cb(evutil_socket_t listener, short event, void *arg)
{
	//传入的event_base指针
	struct event_base *base = (struct event_base*)arg;
	//socket描述符
	evutil_socket_t fd;
	//声明地址
	struct sockaddr_in sin;
	//地址长度声明
	socklen_t slen = sizeof(sin);
	//接收客户端
	fd = accept(listener, (struct sockaddr *)&sin, &slen);
	if (fd < 0)
	{
		perror("error accept");
		return;
	}
	printf("ACCEPT: fd = %u\n", fd);
	////注册一个bufferevent_socket_new事件
	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	////设置回掉函数
	bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
	////设置该事件的属性
	bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
	addClient(fd, bev);
}
////read 回调函数
void read_cb(struct bufferevent *bev, void *arg)
{
#define MAX_LINE 256
	char line[MAX_LINE + 1];
	int n;
	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//
	while (n = bufferevent_read(bev, line, MAX_LINE))
	{
		line[n] = '\0';
		printf("fd=%u, read line: %s,len: %d\n", fd, line,n);
		//将获取的数据返回给客户端
		//bufferevent_write(bev, line, n);
		BoardMessage(line, n);
	}
}
////error回调函数
void error_cb(struct bufferevent *bev, short event, void *arg)
{
	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//cout << "fd = " << fd << endl;
	if (event & BEV_EVENT_TIMEOUT)
	{
		printf("Timed out\n"); //if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}
	removeClient(bev);
	bufferevent_free(bev);
}
////write 回调函数
void write_cb(struct bufferevent *bev, void *arg)
{
	char str[50];
	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//cin >> str;
	printf("输入数据！");
	scanf_s("%d", &str);
	bufferevent_write(bev, &str, sizeof(str));
}

//#define ___UDP___
#define ___TCP___
int main()
{
	int ret;
	evutil_socket_t listener;
	WSADATA  Ws;
	//Init Windows Socket
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		return -1;
	}
	
#ifdef ___UDP___
	listener = socket(PF_INET, SOCK_DGRAM, 0); //socket(PF_INET, SOCK_DGRAM, 0); //
#endif
#ifdef ___TCP___
	listener = socket(AF_INET, SOCK_STREAM, 0);
#endif


	assert(listener > 0);
	evutil_make_listen_socket_reuseable(listener);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(LISTEN_PORT);
	if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("bind");
		return 1;
	}
	if (listen(listener, 1000) < 0) {
		perror("listen");
		return 1;
	}
	printf("Listening...\n");
	evutil_make_socket_nonblocking(listener);
	struct event_base *base = event_base_new();
	assert(base != NULL);
	struct event *listen_event;
	listen_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept_cb, (void*)base);
	event_add(listen_event, NULL);
	event_base_dispatch(base);
	printf("The End.");
	return 0;
}
#endif