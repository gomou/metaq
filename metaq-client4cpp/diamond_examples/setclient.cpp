/*
 * $Id: setclient.cpp 510 2011-09-28 01:47:01Z shijia.wxr $
 */
#include <stdio.h>
#include <DIAMOND/DiamondClient.h>

int main(int argc, char **argv)
{
	if(argc != 4)
	{
		printf("Useage: %s dataId groupId content\n", argv[0]);
		return -1;
	}

	const char* dataId = argv[1];
	const char* groupId = argv[2];
	const char* content = argv[3];

	DIAMOND::DiamondClient dc;

	std::string error;
	if(dc.setConfig(dataId, groupId, content, error))
	{
		printf("setConfig OK, [%s] [%s] [%s]\n", dataId, groupId, content);
	}
	else
	{
		printf("setConfig Failed, [%s] [%s] error:[%s]\n", dataId, groupId, error.c_str());
	}

	return 0;
}
