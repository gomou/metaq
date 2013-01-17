/*
 * $Id: getclient.cpp 510 2011-09-28 01:47:01Z shijia.wxr $
 */
#include <stdio.h>
#include <DIAMOND/DiamondClient.h>

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("Useage: %s dataId groupId\n", argv[0]);
		return -1;
	}

	const char* dataId = argv[1];
	const char* groupId = argv[2];

	DIAMOND::DiamondClient dc;

	std::string content;
	std::string error;
	if(dc.getConfig(dataId, groupId, content, error))
	{
		printf("getConfig OK, [%s] [%s] [%s]\n", dataId, groupId, content.c_str());
	}
	else
	{
		printf("getConfig Failed, [%s] [%s] error:[%s]\n", dataId, groupId, error.c_str());
	}

	return 0;
}
