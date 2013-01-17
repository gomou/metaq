/*
 * $Id: DiamondClient.cpp 776 2011-12-12 11:49:46Z shijia.wxr $
 */
#include "DiamondClient.h"
#include "UtilModule.h"
#include <vector>
#include <lwpr.h>
#include <stdlib.h>
#include <time.h>

namespace DIAMOND
{

#define	DIAMOND_REPOSITORY_ADDR           "diamond.repository.addr"
#define	DIAMOND_INVOKE_TIMEOUT            "diamond.invoke.timeout"
#define	DIAMOND_CLUSTERNAME               "diamond.clusterName"
#define	DIAMOND_LOCALCONFIGDIR            "diamond.localConfigDir"
#define	DIAMOND_POLLING_INTERVAL          "diamond.polling.interval"

#define LINE_SEPARATOR                    ((char)1)
#define WORD_SEPARATOR                    ((char)2)

	SubscriberListener::SubscriberListener()
	{
	}

	SubscriberListener::~SubscriberListener()
	{
	}

	class DiamondClientImpl;

	typedef struct ConfigKey
	{
		std::string dataId;
		std::string groupId;
		bool operator < (const ConfigKey& right) const
		{
			return this->dataId < right.dataId
			       && this->groupId < right.groupId;
		}
	} ConfigKey;

	typedef struct
	{
		SubscriberListener* listener;
		std::string contentMd5;
	} ConfigListener;

	typedef struct
	{
		std::string contentBody;
		std::string contentMd5;
	} ConfigValue;

	class DiamondClientImpl
	{
		typedef std::vector<std::string> DiamondAddressVector;

		LWPR::Mutex m_Mutex;

		// 存储WebServer信息（ConfigServer机器）
		static const char* const SERVER_ADDR_TABLE[2];
		unsigned int m_nWebServerIndex;

		// 存储获取的Diamond信息
		DiamondAddressVector m_DiamondAddressVector;
		LWPR::AtomicInteger m_nDiamondServerIndex;

		volatile bool m_bDiamondPrepared;

		DiamondConfig m_DiamondConfig;


		class PollThread : public LWPR::Thread, public LWPR::Resource
		{
			LWPR::Mutex m_Mutex;

			std::map<ConfigKey, ConfigListener> m_Listeners;

			DiamondClientImpl* m_pDiamondClientImpl;

		public:
			PollThread(DiamondClientImpl* pImpl) : m_pDiamondClientImpl(pImpl)
			{
				assert(NULL != pImpl);
				this->Start();
			}

			~PollThread()
			{
			}

			void shutdown()
			{
				this->StopRunning();
				this->Notify();
				this->Join();
			}

			void registerListener(const char* dataId, const char* groupId, SubscriberListener* listener)
			{
				assert(NULL != dataId);
				assert(NULL != groupId);
				assert(NULL != listener);

				ConfigKey key = {dataId, groupId};
				ConfigListener cflis = {listener, ""};

				LWPR::Synchronized syn(m_Mutex);
				m_Listeners[key] = cflis;
			}

			bool BuildPostFileds(std::string& postFileds)
			{
				LWPR::Synchronized syn(m_Mutex);

				postFileds = "Probe-Modify-Request=";

				for(std::map<ConfigKey, ConfigListener>::iterator it = m_Listeners.begin();
				    it != m_Listeners.end();
				    it++)
				{
					char buf[512] = {0};

					sprintf(buf, "%s%c%s%c%s%c"
					        , it->first.dataId.c_str()
					        , WORD_SEPARATOR
					        , it->first.groupId.c_str()
					        , WORD_SEPARATOR
					        , it->second.contentMd5.c_str()
					        , LINE_SEPARATOR);

					postFileds += buf;
				}

				return !m_Listeners.empty();
			}

			virtual void Run()
			{
				int interval = atoi(this->m_pDiamondClientImpl->m_DiamondConfig[DIAMOND_POLLING_INTERVAL].c_str());
				if(interval <= 0)
				{
					interval = 15;
				}

				while(this->IsContinue())
				{
					this->Wait(interval);

					std::string postFileds;
					if(BuildPostFileds(postFileds))
					{
						std::map<ConfigKey, ConfigValue> configChanaged;
						if(this->m_pDiamondClientImpl->DoPoll(postFileds.c_str(), configChanaged))
						{
							for(std::map<ConfigKey, ConfigValue>::iterator it = configChanaged.begin();
							    it != configChanaged.end();
							    it++)
							{
								LWPR::Synchronized syn(m_Mutex);

								std::map<ConfigKey, ConfigListener>::iterator listenerIt = m_Listeners.find(it->first);
								if(listenerIt != m_Listeners.end())
								{
									try
									{
										listenerIt->second.listener->configOnChanged(it->first.dataId.c_str()
										        , it->first.groupId.c_str()
										        , it->second.contentBody.c_str());
									}
									catch(...)
									{
									}

									listenerIt->second.contentMd5 = it->second.contentMd5;
								}
							}
						}
					}
				}
			}
		};

		DECLAREVAR(PollThread)

		// 轮询部分
		PollThread_var m_vPollThread;

	public:
		DiamondClientImpl(const DiamondConfig* config)
			: m_nWebServerIndex(0)
			, m_nDiamondServerIndex(rand())
			, m_bDiamondPrepared(false)
		{
			m_DiamondConfig[DIAMOND_REPOSITORY_ADDR] = "NONE";
			m_DiamondConfig[DIAMOND_CLUSTERNAME] = "basestone";
			m_DiamondConfig[DIAMOND_INVOKE_TIMEOUT] = "10";
			m_DiamondConfig[DIAMOND_LOCALCONFIGDIR] = getenv("HOME");
			m_DiamondConfig[DIAMOND_LOCALCONFIGDIR] += "/diamond";
			m_DiamondConfig[DIAMOND_POLLING_INTERVAL] = "15";

			if(config)
			{
				DiamondConfig::const_iterator it = config->begin();
				for(; it != config->end(); it++)
				{
					if(it->second.length() > 0)
					{
						m_DiamondConfig[it->first] = it->second;
					}
				}
			}
		}

		~DiamondClientImpl()
		{
			if(m_vPollThread)
			{
				m_vPollThread->shutdown();
			}

			m_vPollThread = NULL;
		}

		bool getConfig(const char* dataId, const char* groupId, std::string& content, std::string& error)
		{
			if(NULL == dataId
			   || NULL == groupId
			   || strlen(dataId) == 0
			   || strlen(groupId) == 0)
			{
				error = "getConfig(): 传入参数错误";
				return false;
			}

			try
			{
				// 首先从用户配置文件中查找
				// 然后尝试从Diamond服务器获取
				// 最后从本地Snapshot获取
				std::string md5;
				return getLocalConfig("data", dataId, groupId, content, error)
				       || getRemoteConfig(dataId, groupId, md5, content, error)
				       || getLocalConfig("snapshot", dataId, groupId, content, error);
			}
			catch(...)
			{
				error = "发生未处理异常";
				return false;
			}

			// 不可能走到这里
			error = "UNKNOW";
			return false;
		}

		bool setConfig(const char* dataId, const char* groupId, const char* content, std::string& error)
		{
			if(NULL == dataId
			   || NULL == groupId
			   || NULL == content
			   || strlen(dataId) == 0
			   || strlen(groupId) == 0
			   || strlen(content) == 0)
			{
				error = "setConfig(): 传入参数错误";
				return false;
			}

			int timeout = atoi(m_DiamondConfig[DIAMOND_INVOKE_TIMEOUT].c_str());
			time_t begintime = time(NULL);

			try
			{
				m_bDiamondPrepared = m_bDiamondPrepared || fetchRemoteDiamondServerList() || fetchLocalDiamondServerList();
				if(!m_bDiamondPrepared)
				{
					error = "从ConfigServer获取Diamond地址列表失败";
					return false;
				}

				std::string strDataId;
				std::string strGroupId;
				std::string strContent;
				if(!(Util::HttpEncode(dataId, 0, strDataId)
				     && Util::HttpEncode(groupId, 0, strGroupId)
				     && Util::HttpEncode(content, 0, strContent)))
				{
					error = "http url encode error";
					return false;
				}

				DiamondAddressVector diamonds;
				if(getDiamondServerList(diamonds))
				{
					int index = m_nDiamondServerIndex;

					for(size_t i = 0;
					    i < diamonds.size();
					    i++)
					{
						std::string addr = diamonds[(unsigned int)index % diamonds.size()];

						char httpURL[512] = {0};
						sprintf(httpURL, "http://%s:8080/diamond-server/basestone.do?method=updateAll", addr.c_str());

						Util::HTTPReqHeader requestHeader;
						Util::HTTPRepHeader responseHeader;
						std::string responseBody;

						Util::NameValuePair postFields;
						postFields["dataId"] = strDataId;
						postFields["group"] = strGroupId;
						postFields["content"] = strContent;

						bool result = Util::PerformHttpPostRequest(httpURL, requestHeader, postFields, timeout, responseHeader, responseBody);
						if(result && responseHeader.code == 200)
						{
							m_nDiamondServerIndex = index;
							return true;
						}
						else if((time(NULL) - begintime) > timeout)
						{
							error = "操作超时";
							m_nDiamondServerIndex = ++index;
							return false;
						}
						else
						{
							index++;
						}
					}

					error = "所有DIAMOND都不可用";
					return false;
				}
			}
			catch(...)
			{
				error = "发生未处理异常";
				return false;
			}

			// 不可能走到这里
			error = "UNKNOW";
			return false;
		}

		bool registerListener(const char* dataId, const char* groupId, SubscriberListener* listener)
		{
			bool result = dataId && groupId && listener;
			if(result)
			{
				std::string newContent;
				std::string error;
				if(getLocalConfig("data", dataId, groupId, newContent, error))
				{
					listener->configOnChanged(dataId, groupId, newContent.c_str());
					return true;
				}

				if(NULL == m_vPollThread)
				{
					m_vPollThread = new PollThread(this);
				}

				m_vPollThread->registerListener(dataId, groupId, listener);
			}

			return result;
		}

		bool DoPoll(const char* postFileds, std::map<ConfigKey, ConfigValue>& configChanaged)
		{
			// 从DiamondServer获取值变化了的DataID列表
			std::vector<ConfigKey> keyFields;
			if(getRemoteConfig(postFileds, keyFields))
			{
				// 获取具体变化值
				for(size_t i = 0; i < keyFields.size(); i++)
				{
					ConfigValue value;
					std::string error;
					if(getRemoteConfig(keyFields[i].dataId.c_str(), keyFields[i].groupId.c_str(), value.contentMd5, value.contentBody, error))
					{
						configChanaged[keyFields[i]] = value;
					}
				}
			}

			return true;
		}

	private:

		bool fetchRemoteDiamondServerList(const char* webserver)
		{
			assert(NULL != webserver);

			char httpURL[512] = {0};

			sprintf(httpURL, "http://%s/diamond-server/%s", webserver, m_DiamondConfig[DIAMOND_CLUSTERNAME].c_str());

			Util::HTTPReqHeader requestHeader;
			Util::HTTPRepHeader responseHeader;
			std::string responseBody;
			int timeout = atoi(m_DiamondConfig[DIAMOND_INVOKE_TIMEOUT].c_str());
			bool result = Util::PerformHttpGetRequest(httpURL, requestHeader, timeout, responseHeader, responseBody);
			if(result)
			{
				m_DiamondAddressVector.clear();
				LWPR::StringUtil::StringLinesToVector(responseBody.c_str(), m_DiamondAddressVector);
			}

			return result && !m_DiamondAddressVector.empty();
		}

		bool fetchLocalDiamondServerList()
		{
			std::string path = m_DiamondConfig[DIAMOND_LOCALCONFIGDIR];
			path += "/";
			path += m_DiamondConfig[DIAMOND_CLUSTERNAME];
			path += "/ServerAddress";

			LWPR::Buffer buf;
			if(Util::LoadFileToBuffer(path.c_str(), buf))
			{
				LWPR::Synchronized syn(m_Mutex);
				m_DiamondAddressVector.clear();
				LWPR::StringUtil::StringLinesToVector(buf.Inout(), m_DiamondAddressVector);
				return !m_DiamondAddressVector.empty();
			}

			return false;
		}

		bool fetchRemoteDiamondServerList()
		{
			LWPR::Synchronized syn(m_Mutex);

			if(m_bDiamondPrepared)
				return true;

			bool result = false;
			std::string rep = m_DiamondConfig[DIAMOND_REPOSITORY_ADDR];

			if("NONE" == rep)
			{
				int size = sizeof(SERVER_ADDR_TABLE) / sizeof(const char * const);
				for(int i = 0;
				    i < size;
				    i++)
				{
					const char* const paddr = SERVER_ADDR_TABLE[(unsigned int)m_nWebServerIndex % size];
					result = fetchRemoteDiamondServerList(paddr);
					if(result)
					{
						break;
					}
					else
					{
						m_nWebServerIndex++;
					}
				}
			}
			else
			{
				result = fetchRemoteDiamondServerList(rep.c_str());
			}

			if(result)
			{
				saveDiamondServerAddress();
			}

			return result;
		}

		bool getDiamondServerList(DiamondAddressVector& addrs)
		{
			LWPR::Synchronized syn(m_Mutex);
			addrs = m_DiamondAddressVector;
			return !addrs.empty();
		}

		/**
		 * 保存从diamond获取的最新配置
		 */
		void saveSnapshotFile(const char* dataId, const char* groupId, const std::string& content)
		{
			assert(NULL != dataId);
			assert(NULL != groupId);

			std::string path = m_DiamondConfig[DIAMOND_LOCALCONFIGDIR];
			path += "/";
			path += m_DiamondConfig[DIAMOND_CLUSTERNAME];
			path += "/snapshot/config-data/";
			path += groupId;
			path += "/";
			path += dataId;

			LWPR::FileUtil::BuildFile(path.c_str());

			FILE* fp = fopen(path.c_str(), "w");
			if(fp)
			{
				fprintf(fp, "%s", content.c_str());
				fclose(fp);
			}
		}

		/**
		 * 保存从全局机器获取的Diamond地址列表
		 */
		void saveDiamondServerAddress()
		{
			if(!m_DiamondAddressVector.empty())
			{
				std::string path = m_DiamondConfig[DIAMOND_LOCALCONFIGDIR];
				path += "/";
				path += m_DiamondConfig[DIAMOND_CLUSTERNAME];
				path += "/ServerAddress";

				LWPR::FileUtil::BuildFile(path.c_str());

				FILE* fp = fopen(path.c_str(), "w");
				if(fp)
				{
					for(size_t i = 0; i < m_DiamondAddressVector.size(); i++)
					{
						fprintf(fp, "%s\n", m_DiamondAddressVector[i].c_str());
					}

					fclose(fp);
				}
			}
		}

		bool getLocalConfig(const char* localPath, const char* dataId, const char* groupId, std::string& content, std::string& error)
		{
			assert(NULL != dataId);
			assert(NULL != groupId);
			assert(NULL != localPath);

			std::string path = m_DiamondConfig[DIAMOND_LOCALCONFIGDIR];
			path += "/";
			path += m_DiamondConfig[DIAMOND_CLUSTERNAME];
			path += "/";
			path += localPath;
			path += "/config-data/";
			path += groupId;
			path += "/";
			path += dataId;

			LWPR::Buffer buf;
			if(Util::LoadFileToBuffer(path.c_str(), buf))
			{
				content = buf.Inout();
				return true;
			}

			if(error.length() == 0)
			{
				error = "加载本地配置文件失败: " + path;
			}

			return false;
		}
#define CONTENT_MD5 "Content-MD5"
#define CLIENT_VERSION_HEADER   "Client-Version"
#define CLIENT_VERSION   "2.0.5"

		bool getRemoteConfig(const char* dataId, const char* groupId, std::string& md5, std::string& content, std::string& error)
		{
			assert(NULL != dataId);
			assert(NULL != groupId);

			int timeout = atoi(m_DiamondConfig[DIAMOND_INVOKE_TIMEOUT].c_str());
			time_t begintime = time(NULL);

			m_bDiamondPrepared = m_bDiamondPrepared || fetchRemoteDiamondServerList() || fetchLocalDiamondServerList();
			if(!m_bDiamondPrepared)
			{
				error = "从ConfigServer获取Diamond地址列表失败";
				return false;
			}

			DiamondAddressVector diamonds;
			if(getDiamondServerList(diamonds))
			{
				int index = m_nDiamondServerIndex;

				for(size_t i = 0;
				    i < diamonds.size();
				    i++)
				{
					std::string addr = diamonds[(unsigned int)index % diamonds.size()];

					char httpURL[512] = {0};
					sprintf(httpURL, "http://%s:8080/diamond-server/config.co?dataId=%s&group=%s"
					        , addr.c_str()
					        , dataId
					        , groupId);

					Util::HTTPReqHeader requestHeader;
					Util::HTTPRepHeader responseHeader;
					bool result = Util::PerformHttpGetRequest(httpURL, requestHeader, timeout, responseHeader, content);
					if(result && responseHeader.code == 200)
					{
						m_nDiamondServerIndex = index;
						saveSnapshotFile(dataId, groupId, content);
						md5 = responseHeader.items[CONTENT_MD5];
						return true;
					}
					else if(result && responseHeader.code == 404)
					{
						m_nDiamondServerIndex = index;

						char buf[512] = {0};
						sprintf(buf, "未找到指定配置, URL = [%s]", httpURL);
						error = buf;
						return false;
					}
					else if((time(NULL) - begintime) > timeout)
					{
						error = "操作超时";
						m_nDiamondServerIndex = ++index;
						return false;
					}
					else
					{
						index++;
					}
				}

				error = "所有DIAMOND都不可用";
				return false;
			}

			error = "调用ConfigServer成功，但是未找到Diamond地址列表";
			return false;
		}

		void convertContent2keyFields(std::string& content, std::vector<ConfigKey>& keyFields)
		{
			if(content != "OK\n" && Util::HttpDecode(content.c_str(), 0, content))
			{
				std::vector<std::string> items;
				std::string::size_type oldPos = 0;
				for(std::string::size_type newPos = content.find_first_of(LINE_SEPARATOR, 0);
				    newPos != std::string::npos;
				    newPos = content.find_first_of(LINE_SEPARATOR, oldPos))
				{
					std::string word = content.substr(oldPos, newPos - oldPos);
					if(word.length() > 0)
					{
						items.push_back(word);
					}

					oldPos = newPos + 1;
				}

				for(size_t i = 0; i < items.size(); i++)
				{
					std::string::size_type pos = items[i].find_first_of(WORD_SEPARATOR, 0);
					if(pos != std::string::npos)
					{
						ConfigKey key = {items[i].substr(0, pos), items[i].substr(pos + 1, items[i].length() - pos - 1)};
						keyFields.push_back(key);
					}
				}
			}
		}

		bool getRemoteConfig(const char* postFields, std::vector<ConfigKey>& keyFields)
		{
			int timeout = atoi(m_DiamondConfig[DIAMOND_INVOKE_TIMEOUT].c_str());
			time_t begintime = time(NULL);

			m_bDiamondPrepared = m_bDiamondPrepared || fetchRemoteDiamondServerList() || fetchLocalDiamondServerList();
			if(!m_bDiamondPrepared)
			{
				fprintf(stderr, "从ConfigServer获取Diamond地址列表失败");
				return false;
			}

			DiamondAddressVector diamonds;
			if(getDiamondServerList(diamonds))
			{
				int index = m_nDiamondServerIndex;

				for(size_t i = 0;
				    i < diamonds.size();
				    i++)
				{
					std::string addr = diamonds[(unsigned int)index % diamonds.size()];

					char httpURL[512] = {0};
					sprintf(httpURL, "http://%s:8080/diamond-server/config.co", addr.c_str());

					Util::HTTPReqHeader requestHeader;
					Util::HTTPRepHeader responseHeader;
					std::string content;
					requestHeader[CLIENT_VERSION_HEADER] = CLIENT_VERSION;
					bool result = Util::PerformHttpPostRequest(httpURL, requestHeader, postFields, timeout, responseHeader, content);
					if(result && responseHeader.code == 200)
					{
						m_nDiamondServerIndex = index;
						convertContent2keyFields(content, keyFields);
						return true;
					}
					else if(result && responseHeader.code == 404)
					{
						m_nDiamondServerIndex = index;
						fprintf(stderr, "未找到指定配置, URL = [%s]", httpURL);
						return false;
					}
					else if((time(NULL) - begintime) > timeout)
					{
						fprintf(stderr, "操作超时");
						m_nDiamondServerIndex = ++index;
						return false;
					}
					else
					{
						index++;
					}
				}

				fprintf(stderr, "所有DIAMOND都不可用");
				return false;
			}

			fprintf(stderr, "调用ConfigServer成功，但是未找到Diamond地址列表");
			return false;
		}
	};

	const char* const DiamondClientImpl::SERVER_ADDR_TABLE[2] =
	{
		"commonconfig.config-host.taobao.com:8080",//线上
		"commonconfig.taobao.net:8080",//日常
	};

	DiamondClient::DiamondClient(const DiamondConfig* config /*= NULL*/) : m_Impl(new DiamondClientImpl(config))
	{

	}

	DiamondClient::~DiamondClient()
	{
		delete m_Impl;
		m_Impl = NULL;
	}

	bool DiamondClient::registerListener(const char* dataId, const char* groupId, SubscriberListener* listener)
	{
		return m_Impl->registerListener(dataId, groupId, listener);
	}

	bool DiamondClient::getConfig(const char* dataId, const char* groupId, std::string& content)
	{
		std::string error;
		return m_Impl->getConfig(dataId, groupId, content, error);
	}

	bool DiamondClient::getConfig(const char* dataId, const char* groupId, std::string& content, std::string& error)
	{
		if(m_Impl->getConfig(dataId, groupId, content, error))
		{
			error.clear();
			return true;
		}

		return false;
	}

	bool DiamondClient::setConfig(const char* dataId, const char* groupId, const char* content)
	{
		std::string error;
		return m_Impl->setConfig(dataId, groupId, content, error);
	}

	bool DiamondClient::setConfig(const char* dataId, const char* groupId, const char* content, std::string& error)
	{
		if(m_Impl->setConfig(dataId, groupId, content, error))
		{
			error.clear();
			return true;
		}

		return false;
	}
}
