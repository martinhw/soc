#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "SocketServer.h"

//本文件是服务器的代码
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>     // for socket

#include <sys/socket.h>    // for socket
#include <string.h>        // for bzero
#include <time.h>          // for time_t and time
#include <unistd.h>        // for close socket
#include <errno.h>	   // for socket error
#include <arpa/inet.h>

#define HELLO_WORLD_SERVER_PORT 9000
#define LENGTH_OF_LISTEN_QUEUE 60

//设置一个socket地址结构server_addr,代表服务器internet地址, 端口
struct sockaddr_in server_addr;
//int server_socket;

SocketServer::SocketServer()
{
	this->port=HELLO_WORLD_SERVER_PORT;
	this->socket_id=0;
}
SocketServer::SocketServer(int port)
{
	this->port=port;
	this->socket_id=0;
}

SocketServer::~SocketServer(void)
{
}

bool SocketServer::start(){

    //设置一个socket地址结构server_addr,代表服务器internet地址, 端口
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    int opt = 1;
    socklen_t len = sizeof(opt);
    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
    mSocket = socket(AF_INET,SOCK_STREAM,0);
  
    if( mSocket < 0)
    {
	close(mSocket); //关闭套接字
	std::cout << "Create Socket Failed!" << endl;
	return false;
    }

    //配置侦听Socket
    //SO_REUSEADDR BOOL 允许套接口和一个已在使用中的地址捆绑。
    if(setsockopt(mSocket,SOL_SOCKET,SO_REUSEADDR,&opt,len)<0){
	close(mSocket); //关闭套接字
	return false;
    }

    //把socket和socket地址结构联系起来
    if( bind(mSocket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
	close(mSocket); //关闭套接字
    	std::cout << "Server Bind Port : " << HELLO_WORLD_SERVER_PORT << " Failed!" << endl;
	return false;
    }

    //mSocket用于监听
    if ( listen(mSocket, LENGTH_OF_LISTEN_QUEUE) )
    {
	close(mSocket); //关闭套接字
    	std::cout << "Server Listen Failed!" << endl;
	return false;
    }


    while(1)   //循环检测和接受客户端的连接  
    {
	struct sockaddr_in client_addr;
	socklen_t length = sizeof(client_addr);

	int ConnectSocket = accept(mSocket,(struct sockaddr*)&client_addr,&length);
	if ( ConnectSocket < 0)
	{
	    printf("Server Accept Failed!\n");
	    break;
	}


	char addr_p[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&client_addr.sin_addr,addr_p,sizeof(addr_p));
        std::cout << "client IP is " << addr_p << ", port is " << ntohs(client_addr.sin_port) << endl;
	string ip = addr_p;
	socket_id++;	

	SocketThread *client=new SocketThread(socket_id,ConnectSocket,ip);
	this->sockets[socket_id]=client;
	client->start();

    }
    close(mSocket); //关闭套接字
}



/*
bool SocketServer::send(int socket,char* message){
	map<int, SocketThread*>::iterator l_it;
	l_it = this->sockets.find(socket);
	if(l_it != this->sockets.end()){
		return l_it->second->m_send(message);
	}
	return false;
}
*/
bool SocketServer::closea(){
	map<int, SocketThread*>::iterator l_it;
	l_it = this->sockets.begin();
	for( ; l_it!=this->sockets.end(); l_it++ ){
		l_it->second->closeb();
	}
	this->sockets.clear();
	return true;
}
