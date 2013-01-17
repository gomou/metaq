/*
 * $Id: clientlistener.cpp 772 2011-12-12 09:07:33Z shijia.wxr $
 */
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <DIAMOND/DiamondClient.h>

class SubscriberListenerImpl : public DIAMOND::SubscriberListener
{
public:
	virtual void configOnChanged(const char* dataId, const char* groupId, const char* newContent)
	{
		assert(NULL != dataId);
		assert(NULL != groupId);
		assert(NULL != newContent);

		printf("In configOnChanged method: [%s] [%s] [%s]\n", dataId, groupId, newContent);
	}
};

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("Useage: %s dataId groupId\n", argv[0]);
		return -1;
	}

	const char* dataId = argv[1];
	const char* groupId = argv[2];

	DIAMOND::DiamondConfig config;
	config["diamond.repository.addr"] = "commonconfig.taobao.net:8080"; // 日常
	config["diamond.polling.interval"] = "5";// 轮询间隔，单位秒
	//config["diamond.repository.addr"] = "commonconfig.config-host.taobao.com:8080"; // 线上

	DIAMOND::DiamondClient* pDiamond = new DIAMOND::DiamondClient(&config);
	SubscriberListenerImpl* pListener = new SubscriberListenerImpl();

	pDiamond->registerListener(dataId, groupId, pListener);
	pDiamond->registerListener("b", "b", pListener);

	sleep(60);

	delete pDiamond;
	delete pListener;
	pDiamond = NULL;
	pListener = NULL;
	return 0;
}
