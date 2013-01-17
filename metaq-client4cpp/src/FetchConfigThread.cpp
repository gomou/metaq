/*
 * $Id: FetchConfigThread.cpp 12 2011-08-25 05:47:29Z  $
 */
#include "FetchConfigThread.h"
#include "UtilModule.h"
#include <assert.h>
#include <stdlib.h>

namespace META
{

	//----------------------------------------------------------------------
	// class ZKConfig
	//----------------------------------------------------------------------
	ZKConfig::ZKConfig()
		: m_nzkSessionTimeoutMs(6000)
		, m_nzkConnectionTimeoutMs(30000)
		, m_nzkSyncTimeMs(5000)
	{

	}

	ZKConfig::~ZKConfig()
	{

	}

	void ZKConfig::Reset(const ZKConfigVector& cfgs)
	{
		LWPR::Buffer bufName(256);
		LWPR::Buffer bufValue(1024 * 10);

		for(size_t i = 0; i < cfgs.size(); i++)
		{
			bufName.Reset();
			bufValue.Reset();

			// 分割成name/value
			int code = sscanf(cfgs[i].c_str(), "%[^ =]%*[ =]%[^$]", bufName.Inout(), bufValue.Inout());
			if(code != 2)
			{
				continue;
			}

			LWPR::StringUtil::TrimAll(bufName.Inout());
			LWPR::StringUtil::TrimAll(bufValue.Inout());

			if(!strcmp(bufName.Inout(), "zk.zkConnect"))
			{
				m_ConnectHosts.clear();
				LWPR::StringUtil::SplitString(bufValue.Inout(), ',', m_ConnectHosts);
			}
			else if(!strcmp(bufName.Inout(), "zk.zkSessionTimeoutMs"))
			{
				m_nzkSessionTimeoutMs = atoi(bufValue.Inout());
			}
			else if(!strcmp(bufName.Inout(), "zk.zkConnectionTimeoutMs"))
			{
				m_nzkConnectionTimeoutMs = atoi(bufValue.Inout());
			}
			else if(!strcmp(bufName.Inout(), "zk.zkSyncTimeMs"))
			{
				m_nzkSyncTimeMs = atoi(bufValue.Inout());
			}
		}
	}

	std::string ZKConfig::getZKConnectHosts()
	{
		std::string hosts;

		for(size_t i = 0; i < m_ConnectHosts.size(); i++)
		{
			if(i > 0)
			{
				hosts += ",";
			}

			hosts += m_ConnectHosts[i];

			// 不含有端口号，设置默认端口号
			if(std::string::npos == m_ConnectHosts[i].find(":", 0))
				hosts += ":2181";
		}

		return hosts;
	}

	int ZKConfig::getZKSessionTimeoutMs()
	{
		return m_nzkSessionTimeoutMs;
	}

	int ZKConfig::getZKConnectionTimeoutMs()
	{
		return m_nzkConnectionTimeoutMs;
	}

	int ZKConfig::getZKSyncTimeMs()
	{
		return m_nzkSyncTimeMs;
	}

	bool ZKConfig::equal(const ZKConfig& zk)
	{
		return *this == zk;
	}

	bool ZKConfig::operator==(const ZKConfig& zk)
	{
		return this->m_ConnectHosts == zk.m_ConnectHosts
		       && this->m_nzkSessionTimeoutMs == zk.m_nzkSessionTimeoutMs
		       && this->m_nzkConnectionTimeoutMs == zk.m_nzkConnectionTimeoutMs
		       && this->m_nzkSyncTimeMs == zk.m_nzkSyncTimeMs;
	}

	//----------------------------------------------------------------------
	// class FetchConfigThread
	//----------------------------------------------------------------------
	const char* const FetchConfigThread::SERVER_ADDR_TABLE[2] =
	{
		"commonconfig.config-host.taobao.com:8080",//线上
		"commonconfig.taobao.net:8080",//日常
	};

	FetchConfigThread::FetchConfigThread(MetaClientConfig_ptr config) :
		m_vMetaClientConfig(MetaClientConfig_var::Duplicate(config))
		, m_nDiamondServerIndex(rand())
		, m_nWebServerIndex(0)
		, m_bPublishing(false)
	{
		DEBUG_FUNCTION();

		assert(NULL != config);

		//BrokerParttion bp;
		//BrokerParttionVector bpvt;
		//bp.partition = 0;
		//bp.broker = "10.232.102.184:8123";
		//bpvt.push_back(bp);
		//m_TopicBrokerParttionMap["venus-meta-test"] = bpvt;

		this->EnableJoinable();


		if (m_vMetaClientConfig->isZKEnable())
		{
			m_vZKClient = new ZKClient(m_vMetaClientConfig->getZKConnectString().c_str(), this);

			if(m_vZKClient->init())
			{
				logger->info(LTRACE, "ZKClient connect OK");
			}
			else
			{
				m_vZKClient = NULL;
				logger->warn(LTRACE, "ZKClient connect Failed");
			}
		}
	}

	FetchConfigThread::~FetchConfigThread()
	{
		DEBUG_FUNCTION();
	}

	void FetchConfigThread::Run()
	{
		DEBUG_FUNCTION();

		while(IsContinue())
		{
			if (!m_vMetaClientConfig->isZKEnable())
			{
				// 1 向TOMCAT服务器获取diamond地址
				fetchDiamondServerList();

				// 2 向diamond获取meta服务器列表，以及ZK地址
				fetchZKConfig();
			}

			// 3 向ZK获取PARTION信息
			fetchPartionInfo();

			if(this->m_bPublishing)
			{
				this->m_nResource.Notify();
			}

			this->Wait(60);
		}
	}

	void FetchConfigThread::GetBrokerParttionByTopic(const char* topic, BrokerParttionVector& bp)
	{
		DEBUG_FUNCTION();

		assert(NULL != topic);

		LWPR::Synchronized syn(m_Mutex);
		TopicBrokerParttionMap::iterator it = m_TopicBrokerParttionMap.find(topic);
		if(it != m_TopicBrokerParttionMap.end())
		{
			bp = it->second;
		}
	}

	void FetchConfigThread::publish(const char* topic)
	{
		DEBUG_FUNCTION();

		assert(NULL != topic);

		this->m_bPublishing = true;

		{
			LWPR::Synchronized syn(m_Mutex);
			TopicBrokerParttionMap::iterator it = m_TopicBrokerParttionMap.find(topic);
			if(it == m_TopicBrokerParttionMap.end())
			{
				BrokerParttionVector vt;
				m_TopicBrokerParttionMap[topic] = vt;
			}
		}

		this->Notify();

		this->m_nResource.Wait(15);

		this->m_bPublishing = false;
	}

	void FetchConfigThread::fetchDiamondServerList()
	{
		DEBUG_FUNCTION();

		std::string serverUrl = m_vMetaClientConfig->getConfigServerAddress();

		// 用户指定配置
		if(serverUrl.length() > 0)
		{
			if(!fetchDiamondServerList(serverUrl.c_str()))
			{
				logger->warn(LTRACE, "fetchDiamondServerList [%s] failed, try again after a while", serverUrl.c_str());
			}
			else
			{
				logger->debug(LTRACE, "fetchDiamondServerList [%s] OK", serverUrl.c_str());
			}
		}
		// 用户未指定，尝试使用默认
		else
		{
			int size = sizeof(SERVER_ADDR_TABLE) / sizeof(const char * const);
			for(int i = 0;
			    i < size;
			    i++)
			{
				const char* const paddr = SERVER_ADDR_TABLE[(unsigned int)m_nWebServerIndex % size];
				if(!fetchDiamondServerList(paddr))
				{
					logger->warn(LTRACE, "fetchDiamondServerList [%s] failed, try next immediately", paddr);
					m_nWebServerIndex++;
				}
				else
				{
					logger->debug(LTRACE, "fetchDiamondServerList [%s] OK", paddr);
				}
			}
		}
	}

	bool FetchConfigThread::fetchDiamondServerList(const char* webserver)
	{
		DEBUG_FUNCTION();

		assert(NULL != webserver);

		char httpURL[512] = {0};

		sprintf(httpURL, "http://%s/diamond-server/serveraddress", webserver);

		logger->debug(LTRACE, "fetchDiamondServerList HTTP URL = [%s]", httpURL);

		Util::HTTPReqHeader requestHeader;
		Util::HTTPRepHeader responseHeader;
		std::string responseBody;
		bool result = Util::PerformHttpGetRequest(httpURL, requestHeader, 10, responseHeader, responseBody);
		if(result)
		{
			m_DiamondAddressVector.clear();
			LWPR::StringUtil::StringLinesToVector(responseBody.c_str(), m_DiamondAddressVector);

			for(unsigned int i = 0; i < m_DiamondAddressVector.size(); i++)
			{
				logger->debug(LTRACE, "DiamondServerList -------- %03d %s", i, m_DiamondAddressVector[i].c_str());
			}
		}

		return result;
	}

	void FetchConfigThread::fetchZKConfig()
	{
		DEBUG_FUNCTION();

		if(m_DiamondAddressVector.empty())
		{
			logger->warn(LTRACE, "No diamond server, return");
			return;
		}

		for(size_t i = 0;
		    i < m_DiamondAddressVector.size();
		    i++)
		{
			std::string addr = m_DiamondAddressVector[m_nDiamondServerIndex % m_DiamondAddressVector.size()];

			if(!fetchZKConfig(addr.c_str()))
			{
				logger->warn(LTRACE, "fetchZKConfig [%s] failed, try next immediately", addr.c_str());
				m_nDiamondServerIndex++;
			}
			else
			{
				logger->debug(LTRACE, "fetchZKConfig [%s] OK", addr.c_str());
			}
		}
	}

	bool FetchConfigThread::fetchZKConfig(const char* diamondserver)
	{
		DEBUG_FUNCTION();

		assert(NULL != diamondserver);

		char httpURL[512] = {0};

		sprintf(httpURL, "http://%s:8080/diamond-server/config.co?dataId=%s&group=%s"
		        , diamondserver
		        , m_vMetaClientConfig->getDiamondZKDataId().c_str()
		        , m_vMetaClientConfig->getDiamondZKGroup().c_str());

		logger->debug(LTRACE, "fetchZKConfig HTTP URL = [%s]", httpURL);

		Util::HTTPReqHeader requestHeader;
		Util::HTTPRepHeader responseHeader;
		std::string responseBody;
		requestHeader["Content-MD5"] = m_ContentMD5;
		bool result = Util::PerformHttpGetRequest(httpURL, requestHeader, 10, responseHeader, responseBody);
		if(result)
		{
			// Not Modified
			if(responseHeader.code == 304)
			{
				logger->debug(LTRACE, "ZK info in diamond Not Modified");
			}
			// Not Found
			else if(responseHeader.code == 404)
			{
				logger->warn(LTRACE, "ZK info in diamond Not Found");
			}
			// OK
			else if(responseHeader.code == 200)
			{
				m_ContentMD5 = responseHeader.items["Content-MD5"];

				ZKConfigVector vct;
				LWPR::StringUtil::StringLinesToVector(responseBody.c_str(), vct);

				ZKConfig tmpZK;
				tmpZK.Reset(vct);
				if(!m_ZKConfig.equal(tmpZK))
				{
					m_ZKConfig = tmpZK;
					logger->info(LTRACE, "ZK config changed, updated");

					m_vZKClient = NULL;

					m_vZKClient = new ZKClient(m_ZKConfig.getZKConnectHosts().c_str(), this);

					if(m_vZKClient->init())
					{
						logger->info(LTRACE, "ZKClient connect OK");
					}
					else
					{
						m_vZKClient = NULL;
						logger->warn(LTRACE, "ZKClient connect Failed");
					}
				}

				for(unsigned int i = 0; i < vct.size(); i++)
				{
					logger->debug(LTRACE, "ZKConfig -------- %03d %s", i, vct[i].c_str());
				}
			}
			// error
			else
			{
				logger->error(LTRACE, "ZK info in diamond http error %d", responseHeader.code);
				// 返回false，尝试连接其他diamond
				return false;
			}
		}

		return result;
	}

	void FetchConfigThread::DoWatcherCallback(ZKClient* zk, int type, int state, const char *path)
	{
		DEBUG_FUNCTION();

		logger->info(LTRACE, "ZK Server invoke callback, %d %d %s", type, state, path == NULL ? "NULL" : path);

		fetchPartionInfo();
	}

	void FetchConfigThread::fetchPartionInfo()
	{
		DEBUG_FUNCTION();

		const char* BROKERIDSPATH = "/meta/brokers/ids/";
		const char* BROKERTOPICSPATH = "/meta/brokers/topics/";

		if(m_vZKClient == NULL) return;

		// 获取本地订阅的topic
		StringVector topics;
		getTopics(topics);

		// 定义新的分区信息
		TopicBrokerParttionMap partitionMap;

		for(size_t i = 0; i < topics.size(); i++)
		{
			std::string topicPath = BROKERTOPICSPATH;
			topicPath += topics[i];

			BrokerParttionVector bps;

			NodeVector partitionNodes;
			if(m_vZKClient->getChildren(topicPath.c_str(), partitionNodes, true))
			{
				for(size_t k = 0; k < partitionNodes.size(); k++)
				{
					std::string idsPath = BROKERIDSPATH;
					idsPath += partitionNodes[k];

					std::string brokerAddr;
					if(m_vZKClient->getValue(idsPath.c_str(), brokerAddr, true))
					{
						std::string partitionIDPATH = BROKERTOPICSPATH;
						partitionIDPATH += topics[i] + "/";
						partitionIDPATH += partitionNodes[k];

						std::string partitionID;
						if(m_vZKClient->getValue(partitionIDPATH.c_str(), partitionID, true))
						{
							BrokerParttion bp;
							int partitionCnt = atoi(partitionID.c_str());
							if(brokerAddr.length() > 7)
							{
								bp.broker = brokerAddr.substr(strlen("meta://"));

								for(int m = 0; m < partitionCnt; m++)
								{
									bp.partition = m;

									bps.push_back(bp);

									logger->debug(LTRACE, "fetch parttion record from ZK OK [%s] [%s] [%d]"
									              , topics[i].c_str()
									              , bp.broker.c_str()
									              , bp.partition);
								}
							}
						}
					}
				}

				partitionMap[topics[i]] = bps;
			}
			else
			{
				logger->warn(LTRACE, "getChildren failed PATH = [%s]", topicPath.c_str());
			}
		}

		LWPR::Synchronized syn(m_Mutex);
		m_TopicBrokerParttionMap = partitionMap;
	}

	void FetchConfigThread::getTopics(StringVector & topics)
	{
		DEBUG_FUNCTION();

		LWPR::Synchronized syn(m_Mutex);

		for(TopicBrokerParttionMap::iterator it = m_TopicBrokerParttionMap.begin();
		    it != m_TopicBrokerParttionMap.end(); it++)
		{
			topics.push_back(it->first);
		}
	}

	void FetchConfigThread::shutdown()
	{
		if(hasShutdown()) return;

		Shutdownable::shutdown();

		this->StopRunning();

		this->Notify();

		m_vZKClient = NULL;
	}
}
