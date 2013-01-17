/*
 * $Id: setclient2.cpp 770 2011-12-12 07:20:10Z shijia.wxr $
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

	DIAMOND::DiamondConfig config;
	config["diamond.repository.addr"] = "commonconfig.taobao.net:8080"; // 日常
	//config["diamond.repository.addr"] = "commonconfig.config-host.taobao.com:8080"; // 线上
	DIAMOND::DiamondClient dc(&config);

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
