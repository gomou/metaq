/*
 * $Id: client.cpp 539 2011-09-30 08:06:54Z shijia.wxr $
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <DIAMOND/DiamondClient.h>

typedef std::vector<std::string> NameList;

static NameList g_DataId;
static NameList g_GroupId;
static NameList g_Content;

#define TEST_TOTAL_ITEMS	10

bool CreateConfig(DIAMOND::DiamondClient& diamond)
{
	bool result = true;

	for(int i = 0; i < TEST_TOTAL_ITEMS && result; i++)
	{
		char bufDataId[128] = {0};
		char bufGroupId[128] = {0};
		char bufContent[128] = {0};

		sprintf(bufDataId, "DATAID_%d_%d", i, rand());
		sprintf(bufGroupId, "GROUPID_%d_%d", i, rand());
		sprintf(bufContent, "CONTENT_%d_%d", i, rand());

		std::string error;
		result = result && diamond.setConfig(bufDataId, bufGroupId, bufContent, error);
		printf("CreateConfig setConfig [%s], [%s] [%s] [%s] [%s]\n"
		       , result ? "OK" : "FAILED"
		       , bufDataId, bufGroupId, bufContent, error.c_str());

		g_DataId.push_back(bufDataId);
		g_GroupId.push_back(bufGroupId);
		g_Content.push_back(bufContent);
	}

	return result;
}

bool UpdateConfig(DIAMOND::DiamondClient& diamond)
{
	bool result = true;

	g_Content.clear();

	for(int i = 0; i < TEST_TOTAL_ITEMS && result; i++)
	{
		char bufContent[128] = {0};

		sprintf(bufContent, "CONTENT_%d_%d", i, rand());

		std::string error;
		result = result && diamond.setConfig(g_DataId[i].c_str(), g_GroupId[i].c_str(), bufContent, error);
		printf("UpdateConfig setConfig [%s], [%s] [%s] [%s] [%s]\n"
		       , result ? "OK" : "FAILED"
		       , g_DataId[i].c_str(), g_GroupId[i].c_str(), bufContent, error.c_str());

		g_Content.push_back(bufContent);
	}

	return result;
}

bool FetchExistConfig(DIAMOND::DiamondClient& diamond)
{
	bool result = true;

	for(int i = 0; i < TEST_TOTAL_ITEMS && result; i++)
	{
		std::string error;
		std::string bufContent;
		result = result && diamond.getConfig(g_DataId[i].c_str(), g_GroupId[i].c_str(), bufContent, error);
		printf("FetchExistConfig getConfig [%s]\n", result ? "OK" : "FAILED");

		result = result && bufContent == g_Content[i];
		printf("\tFetchExistConfig compare [%s], [%s] [%s] [%s] [%s] [%s]\n"
		       , result ? "EQUAL" : "NOT EQUAL"
		       , g_DataId[i].c_str(), g_GroupId[i].c_str(), bufContent.c_str(), g_Content[i].c_str(), error.c_str());
	}

	return result;
}

bool FetchNotExistConfig(DIAMOND::DiamondClient& diamond)
{
	bool result = true;

	for(int i = 0; i < TEST_TOTAL_ITEMS && result; i++)
	{
		std::string error;
		std::string bufContent;
		result = result && !diamond.getConfig("DATAID_HELLO_AAAAAAAAAAAA", "GROUPID_HELLO_AAAAAAAAAAAA", bufContent, error);
		printf("FetchNotExistConfig [%s] [%s]\n", result ? "OK" : "FAILED", error.c_str());
	}

	return result;
}

int main(int argc, char **argv)
{
	DIAMOND::DiamondConfig config;
#if 1
	config["diamond.repository.addr"] = "commonconfig.taobao.net:8080"; // 日常
#else
	config["diamond.repository.addr"] = "commonconfig.config-host.taobao.com:8080"; // 线上
#endif

	config["diamond.localConfigDir"] = getenv("HOME");
	config["diamond.localConfigDir"] += "/diamond_test";

	DIAMOND::DiamondClient diamond(&config);

	srand(time(NULL));

	bool result = CreateConfig(diamond);

	sleep(3);

	result = result && FetchExistConfig(diamond);

	sleep(3);

	result = result && UpdateConfig(diamond);

	sleep(3);

	result = result && FetchExistConfig(diamond);

	sleep(3);

	result = result && FetchNotExistConfig(diamond);

	printf("diamond test suite [%s]\n", result ? "OK" : "FAILED");

	return result ? 0 : -1;
}
