#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <map>
#include "SocketThread.h"


//本文件是服务器的代码
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>     // for socket

#include <sys/socket.h>    // for socket
#include <stdio.h>         // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>          // for time_t and time
#include <unistd.h>        // for close socket
#include <errno.h>	   // for socket error


using namespace std;

class SocketServer
{
public:
	SocketServer(int port);
	SocketServer();
	~SocketServer(void);

public:
	int port;

	int socket_id;
	int mSocket;
	map<int,SocketThread*> sockets;

public:
	bool start();//启动
//	bool send(int socket,char* message);//发送
	bool closea();//关闭socket和线程



};
