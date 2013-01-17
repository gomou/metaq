/*
 * $Id: FetchConfigThread.h 12 2011-08-25 05:47:29Z  $
 */
#ifndef _LIBMCLI_FETCH_CONFIG_THREAD_H__
#define _LIBMCLI_FETCH_CONFIG_THREAD_H__

#include <lwpr.h>
#include <map>
#include <vector>
#include <string>

#include "MetaClientConfig.h"
#include "ZKClient.h"
#include "Shutdownable.h"

namespace META
{
	typedef std::string TOPIC_T;
	typedef std::string BROKER_T;
	typedef LWPR::INT32 PARTTION_T;

	typedef struct
	{
		BROKER_T broker;
		PARTTION_T partition;
	} BrokerParttion;

	typedef std::vector<BrokerParttion> BrokerParttionVector;

	typedef std::map<TOPIC_T, BrokerParttionVector> TopicBrokerParttionMap;

	typedef std::vector<std::string> DiamondAddressVector;
	typedef std::vector<std::string> ZKConfigVector;

	typedef std::vector<std::string> ZKConnectHostVector;

	typedef std::vector<std::string> StringVector;


	class ZKConfig
	{
	public:
		ZKConfig();
		~ZKConfig();

		bool operator == (const ZKConfig& zk);

		bool equal(const ZKConfig& zk);

		void Reset(const ZKConfigVector& cfgs);

		std::string getZKConnectHosts();

		int getZKSessionTimeoutMs();

		int getZKConnectionTimeoutMs();

		int getZKSyncTimeMs();

	private:
		ZKConnectHostVector m_ConnectHosts;
		int m_nzkSessionTimeoutMs;
		int m_nzkConnectionTimeoutMs;
		int m_nzkSyncTimeMs;
	};

	class FetchConfigThread 
		: public LWPR::Thread
		, public WatcherCallback
		, public LWPR::Resource
		, public Shutdownable
	{
	public:
		FetchConfigThread(MetaClientConfig_ptr config);
		~FetchConfigThread();

		virtual void Run();

		void GetBrokerParttionByTopic(const char* topic, BrokerParttionVector& bp);

		void publish(const char* topic);

		virtual void DoWatcherCallback(ZKClient* zk, int type, int state, const char *path);

		virtual void shutdown();

	private:
		void fetchDiamondServerList();

		bool fetchDiamondServerList(const char* webserver);

		void fetchZKConfig();

		bool fetchZKConfig(const char* diamondserver);

		void fetchPartionInfo();

		void getTopics(StringVector& topics);

	private:
		LWPR::Mutex m_Mutex;
		TopicBrokerParttionMap m_TopicBrokerParttionMap;

		// 以下数据不需要加锁， 因为只有线程自己访问
		MetaClientConfig_var m_vMetaClientConfig;

		// 存储ZKConfig
		ZKConfig m_ZKConfig;
		ZKClient_var m_vZKClient;
		std::string m_ContentMD5;

		// 存储获取的Diamond信息
		DiamondAddressVector m_DiamondAddressVector;
		unsigned int m_nDiamondServerIndex;

		// 存储WebServer信息（ConfigServer机器）
		static const char* const SERVER_ADDR_TABLE[2];
		unsigned int m_nWebServerIndex;

		// 设置publish异步通知模式
		volatile bool m_bPublishing;
		LWPR::Resource m_nResource;
	};

	DECLAREVAR(FetchConfigThread);
}
#endif // end of _LIBMCLI_FETCH_CONFIG_THREAD_H__
