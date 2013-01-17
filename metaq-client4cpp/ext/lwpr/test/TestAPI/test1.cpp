/*
 * $Id: test1.cpp 3 2011-08-19 02:25:45Z  $
 */
/*
 * 测试各种接口的调用
 */
#include "Socket.h"
#include "ProcessUtil.h"
#include "StringUtil.h"
#include "FileUtil.h"
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

int testHostName2Value(int argc, char *argv[])
{
	std::string ip;

	if (LWPR::Socket::HostName2Value(argv[1], ip))
	{
		cout << "HostName2Value OK" << endl;

		cout << ip << endl;
	}
	else
	{
		cout << "HostName2Value Failed" << endl;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	try
	{
		return testHostName2Value(argc, argv);
	}
	catch(const LWPR::Exception& e)
	{
		fprintf(stderr, "%s\n", e.what());
	}

	return 0;
}
