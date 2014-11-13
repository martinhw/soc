#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>     // for socket

#include <sys/socket.h>    // for socket
#include <stdio.h>         // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>          // for time_t and time
#include <unistd.h>        // for close socket
#include <errno.h>	   // for socket error


#include <arpa/inet.h>
#include <netinet/tcp.h>

//for operation file
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <time.h>
#include <unistd.h>

#include "json/json.h" // for operation json


using namespace std;
typedef std::vector<std::string> vector_t;


#define BUFFER_SIZE 1024
class SocketThread
{
public:
	SocketThread(int id,int socket,string ip);
	~SocketThread(void);
private:

//	bool IsSocketClosed();



public:
	int id;
	string ip;
	int mSocket;


	string macStr;
	string textStr;



	char buffer[BUFFER_SIZE];
	string writeStr;	//标记为从macFile.txt中读出的mac地址
	
	bool start();
	bool m_send(string message);//发送
	bool closeb();




	string readfile();		//读文件，从文件中读取mac地址，并将读取出来的mac地址从原文件中删除掉
	bool modelreadfile(vector_t vt);
	bool modelfile(string str);	//写文件，将mac地址写入一个临时文件中
	bool deletefile(string str);	//删除文件，每次操作完成后，将写入临时文件中的mac地址删除掉
	bool writeDateFile(string str);	//写文件，接收字符串，并将其写入文件中
	bool delDateFile(string str);//
};
