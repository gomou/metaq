/*
 * $Id: test2.cpp 3 2011-08-19 02:25:45Z  $
 */
/*
 * 测试LWPR::Socket类是否能正常工作
 * 充当Client
 */
#include "Socket.h"
#include <iostream>
#include <stdio.h>

using namespace std;

#define LISTEN_PORT		10086

int main(int argc, char *argv[])
{
	LWPR::SOCKET_FD_T nClientUDP = LWPR::Socket::CreateClientUDPSocket();
	if(nClientUDP == -1)
	{
		cout << "CreateClientUDPSocket Failed." << endl;
		return -1;
	}

	LWPR::SOCKET_RET_TYPE_E ret = LWPR::Socket::WriteUDPSocket(nClientUDP, LISTEN_PORT, argv[1], strlen(argv[1]));
	cout << "WriteUDPSocket " << ret << endl;

	return 0;
}



