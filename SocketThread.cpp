#include "SocketThread.h"

/*
#define POSE_IP		"10.0.0.100"
#define POSE_PORT	9000
*/

pthread_mutex_t mutex0;		//macFile.txt
pthread_mutex_t mutex1;		//tempMacFile.txt
pthread_mutex_t mutex2;		//DateFile.txt

SocketThread::SocketThread(int id, int socket, string ip)
{
	this->id=id;
	this->mSocket=socket;
	this->ip=ip;
}

SocketThread::~SocketThread(void)
{
}

/*
int sendVal = 0;
string macStr;
string textStr;
*/
void * ReceiveThread(void * param) 
{
    SocketThread *self=(SocketThread*)param;
//    SocketThread *self= this;

    self->macStr = "";
    self->textStr = "";
    while(1){
	bzero(self->buffer,BUFFER_SIZE);
	//接收客户端发送来的信息到buffer中
	int length = recv(self->mSocket,self->buffer,BUFFER_SIZE,0);

	if (length <= 0)
	{
	    cout << "errno: " << errno << endl;
	    cout << "length: " << length << endl;
	    if(errno == EINTR  || errno == EWOULDBLOCK || errno == EAGAIN)
		cout << "Server Recieve Data From " << self->ip << " is Failed!" << endl;
	    else{
		cout << "The client " << self->ip << " has been disconnected! 00" << endl;
		self->closeb();
	    }
	    return NULL;
	}
	
	std::cout << endl << "Server Receiver Data From " << self->ip << " is:" << self->buffer << endl;

	string test = self->buffer;
	Json::Reader reader;
	Json::Value value;
	if(reader.parse(test,value))
	{
		Json::Value actionJson = value["action"];
		int action = actionJson.asInt();
		string returnJson;
		std::cout << "action:" << action << endl;
		if(action == 0){
			string macStr = self->readfile();
			bool isModelSuccessd = self->modelfile(macStr);
			if(isModelSuccessd)
				returnJson = "{\"action\":0,\"mac\":\""+ macStr+"\"}";
			else
				returnJson = "{\"action\":0,\"mac\":\"\"}";

			bool senda = self->m_send(returnJson);	
			bzero(self->buffer,BUFFER_SIZE);
		}
		else if(action == 1){
			Json::Value macJson = value["mac"];
			Json::Value textJson = value["text"];
			self->macStr = macJson.asString();
			self->textStr = textJson.asString();

			bool isWriteSuccessed = self->writeDateFile(self->textStr);
			if(isWriteSuccessed)
				returnJson = "{\"action\":1,\"ret\":1}";
			else
				returnJson = "{\"action\":1,\"ret\":0}";

			bool isModelSuccessed = self->deletefile(self->macStr);
			if(isModelSuccessed)
				returnJson = "{\"action\":1,\"ret\":1}";
			else
				returnJson = "{\"action\":1,\"ret\":0}";	


			bool sendb = self->m_send(returnJson);	
			bzero(self->buffer,BUFFER_SIZE);

			if( !sendb ){
				std::cout << "****************************" << endl;
				self->modelfile(self->macStr);
				self->delDateFile(self->textStr);
	    			std::cout << "Server Send Data To " << self->ip << " is Failed!" << endl;
//				break;
			}
			else{ //判断客户端是否断开
				struct tcp_info info;
				int len=sizeof(info);
				getsockopt(self->mSocket, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
				if((info.tcpi_state==TCP_ESTABLISHED)){
					cout << "socket connected, and send data success" << endl;
				}
				else{
					cout << "socket disconnected and send data failed" << endl;
					self->modelfile(self->macStr);
					self->delDateFile(self->textStr);
					cout << "The client " << self->ip << " has been disconnected!" << endl;	
				}
			}
		}
	}
	else{
		std::cout << "parse json failed!";
		break;
	}
    }
    return 0; 
}


bool SocketThread::start(){
	pthread_t id;
	int ret;
	ret=pthread_create(&id,NULL,ReceiveThread, this);
	if(ret!=0){
		printf ("Create pthread error!n");
	}
	return true;
}

bool SocketThread::m_send(string message){
	int sendVal = send(this->mSocket,message.c_str(),message.length(),0);

	if(sendVal <= 0){
		if(sendVal < 0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)){
		    std::cout << "Server Send Data To " << this->ip << " is Failed!" << endl;
		    return false;
		}
		else{
		    cout << "The client " << this->ip << " has been disconnected!" << endl;
		}		
	}
	std::cout << "Server Send Data To " << this->ip << " is:" << message << endl;
	return true;
}

bool SocketThread::closeb(){
	shutdown(this->mSocket, SHUT_RDWR);	//关闭socket的读写功能
	close(this->mSocket);
	return true;
}
/*
bool SocketThread::IsSocketClosed()  
{  
	bool ret = false;  
	HANDLE closeEvent = WSACreateEvent();  
	WSAEventSelect(this->mSocket, closeEvent, FD_CLOSE);  
  
	DWORD dwRet = WaitForSingleObject(closeEvent, 0);  
   
	if(dwRet == WSA_WAIT_EVENT_0)
		ret = true;  
	else if(dwRet == WSA_WAIT_TIMEOUT) 
		ret = false;  
  
//	WSACloseEvent(closeEvent);  
	return ret;  
} 
*/





string SocketThread::readfile()
{
    pthread_mutex_lock(&mutex0);
    this->writeStr = "";
    fstream outfile("macFile.txt", ios::in|ios::app);    //定义输入文件类对象	
    if(!outfile)                                   //判断文件打开是否成功
    {
    	cerr<<"Cannot open the macFile.txt!"<<endl;     //如果文件创建或者打开不成功，则打印出错信息
//	exit(1);                             //退出程序

	pthread_mutex_unlock(&mutex0);
	return "";
    }
	
    vector_t data;
    // 读取文件
    int line_num = 0;
    string line = "";
    while(getline(outfile, line)){
        ++line_num;
        string tmp_string;
        std::istringstream tmp_str(line);
        data.push_back(line);
    }	

    outfile.close();


    int n = data.size();
    if(n <=0 ){
	cout << "已没有MAC地址！" << endl;
    	pthread_mutex_unlock(&mutex0);
	return "";
    }
    else{
        if (!this->modelreadfile(data)){
    	    pthread_mutex_unlock(&mutex0);
            return "";
	}
    }
    
    pthread_mutex_unlock(&mutex0);
    return this->writeStr;
}


bool SocketThread::modelreadfile(vector_t vt)
{
    fstream outfile("macFile.txt", ios::out);    //定义输入文件类对象	
    if(!outfile)                                 //判断文件打开是否成功
    {
	cerr<<"Cannot open the macFile.txt!"<<endl;     //如果文件创建或者打开不成功，则打印出错信息

	return false;
    }
	
    int n = vt.size();
    if(n <=0 ){
	std::cout << "已没有MAC地址！" << endl;
	outfile.close();
	return false;
    }
    else{
	this->writeStr = vt[n-1];
	vt.erase(vt.end() - 1);
		
	outfile.seekp(0, ios::beg);
	for (int i = 0; i != vt.size(); ++i){
	    outfile << vt[i] << "\n";
	}
    }
    outfile.close();
    return true;
}

bool SocketThread::modelfile(string str){	
    if(str.size() == 0)
	return true;
    pthread_mutex_lock(&mutex1);
    // 输出到另一个文件
    fstream outfile("tempMacFile.txt", ios::app|ios::in|ios::out);  
    if (!outfile.is_open())   
    {  
        std::cerr << "文件tempMacFile.txt打开失败\n" <<  __LINE__ << endl;
	pthread_mutex_unlock(&mutex1);
        return false;
    }  
    vector_t tempdata;
    string line = "";
    while(getline(outfile, line)){
	tempdata.push_back(line);
    }
	
    outfile.close();	
	
    vector<std::string>::iterator find_t;
    find_t = find(tempdata.begin(), tempdata.end(), str);
    if (find_t != tempdata.end() ){
	std::cout << "the mac is exist in tempMacFile.txt!" << endl;
    }
    else{		
	//没找到		
	std::cout << "the mac " << str << " is not exist in tempMacFile.txt" << endl;
		
	fstream infile("tempMacFile.txt", ios::app|ios::in|ios::out);  
	if(tempdata.size() < 1){
		infile << str;
	}
	else{
		infile << "\n" << str;
	}
	infile.close();
    }

    tempdata.clear();
    pthread_mutex_unlock(&mutex1);

    return true;
}

bool SocketThread::deletefile(string str)
{
    pthread_mutex_lock(&mutex1);
    // 输出到另一个文件
    fstream outfile("tempMacFile.txt", ios::app|ios::in|ios::out);  
    if (!outfile.is_open())   
    {  
        std::cerr << "文件tempMacFile.txt打开失败\n" <<  __LINE__ << endl;
	pthread_mutex_unlock(&mutex1);
        return false;
    }  
    vector_t tempdata;
    string line = "";
    while(getline(outfile, line)){
	tempdata.push_back(line);
    }
	
    outfile.close();

    std::cout << "str : " << str << endl;
    std::cout << "tempdata.end() : " << *(tempdata.end()-1) << endl;
	
    vector<std::string>::iterator find_t;
    find_t = find(tempdata.begin(), tempdata.end(), str);
    if (find_t != tempdata.end() ){
	cout << "the mac " << str << " is exist in tempMacFile.txt, delete it!" << endl;

	tempdata.erase(find_t);				  //删除mac地址

	fstream outtfile("tempMacFile.txt", ios::out);    //定义输入文件类对象	
	outtfile.seekp(0, ios::beg);
	if(tempdata.size() > 0){
		for (int i = 0; i < tempdata.size()-1; ++i){
		    outtfile << tempdata[i] << "\n";
		}
		outtfile << tempdata[tempdata.size()-1];
	}
	outtfile.close();
    }
    else{//没找到		
	std::cout << "the mac " << str << " is not exist in tempMacFile.txt. cannot to delete it" << endl;
    }
    tempdata.clear();
    pthread_mutex_unlock(&mutex1);

    return true;
}


bool SocketThread::writeDateFile(string str)
{	
    string dateStr;
    char szBuf[256] = {0};  
    time_t timer = time(NULL);  
    //strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));  
    strftime(szBuf, sizeof(szBuf), "%Y-%m-%d", localtime(&timer));  
    dateStr = szBuf;
    dateStr += ".txt";
    
    const char *filename = dateStr.c_str();  
    std::cout << "filename:" << filename << endl;


    pthread_mutex_lock(&mutex2);
// 输出到另一个文件
    fstream outfile(filename, ios::in|ios::app);  
    if (!outfile.is_open())   
    {  
        std::cerr << "open the file " << filename <<" is failed!\n" << __LINE__;
	
	pthread_mutex_unlock(&mutex2);
	return false;
    }  
	 
    vector_t outdata;
    string line = "";
    while(getline(outfile, line)){
    	outdata.push_back(line);
    }

    outfile.close();	
    std::cout << "the product data " << str << " is not exist in " << filename << endl;	

    //没找到			
    fstream infile(filename, ios::app|ios::in|ios::out);  
    if(outdata.size() < 1){	
	infile << str;
    }else{	
	infile << "\n" << str;
    }
    infile.close();
    outdata.clear();

    pthread_mutex_unlock(&mutex2);
    return true;
}


bool SocketThread::delDateFile(string str)
{	
    string dateStr;
    char szBuf[256] = {0};  
    time_t timer = time(NULL);  
    //strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));  
    strftime(szBuf, sizeof(szBuf), "%Y-%m-%d", localtime(&timer));  
    dateStr = szBuf;
    dateStr += ".txt";
    
    const char *filename = dateStr.c_str();  
    std::cout << "filename:" << filename << endl;


    pthread_mutex_lock(&mutex2);
// 输出到另一个文件
    fstream outfile(filename, ios::in|ios::app|ios::out);  
    if (!outfile.is_open())   
    {  
        std::cerr << "open the file " << filename <<" is failed!\n" << __LINE__;
	
	pthread_mutex_unlock(&mutex2);
	return false;
    }  
	 
    vector_t outdata;
    string line = "";
    while(getline(outfile, line)){
    	outdata.push_back(line);
    }

    outfile.close();	

    vector<std::string>::iterator find_t;
    find_t = find(outdata.begin(), outdata.end(), str);
    if (find_t != outdata.end() ){
	cout << "the product data " << str << " is exist in " << filename << ", delete it!" << endl;

	outdata.erase(find_t);				  //删除刚刚插入的数据

	fstream outtfile(filename, ios::out);    //定义输入文件类对象	
	outtfile.seekp(0, ios::beg);
	if(outdata.size() > 0){
		for (int i = 0; i < outdata.size()-1; ++i){
		    outtfile << outdata[i] << "\n";
		}
		outtfile << outdata[outdata.size()-1];
	}
	outtfile.close();
    }
    else{//没找到		
	std::cout << "the mac " << str << " is not exist in " << filename << ", cannot to delete it" << endl;
    }
    outdata.clear();

    pthread_mutex_unlock(&mutex2);
    return true;
}

