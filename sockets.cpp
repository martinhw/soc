#include "SocketServer.h"

int main(int argc, char **argv)
{
	int port = 9000;
	SocketServer* ss = new SocketServer(9000);
	while( !ss->start() ){
		ss->closea();
		break;
	}
	return 0;
}

