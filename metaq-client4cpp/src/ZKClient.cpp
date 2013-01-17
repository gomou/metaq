/*
 * $Id$
 */
#include "ZKClient.h"

namespace META
{
	ZKClient::ZKClient(const char* hosts, WatcherCallback_ptr watcher /*= NULL*/)
		: m_hosts(hosts)
		, m_zh(NULL)
		, m_Init(false)
	{
		DEBUG_FUNCTION();

		assert(NULL != hosts);

		if(watcher)
		{
			m_vWatcherCallback = WatcherCallback_var::Duplicate(watcher);
		}

		zoo_set_debug_level (ZOO_LOG_LEVEL_WARN);
	}

	ZKClient::~ZKClient()
	{
		DEBUG_FUNCTION();

		closeZK();
	}

	bool ZKClient::init()
	{
		DEBUG_FUNCTION();

		m_zh = zookeeper_init(m_hosts.c_str(), watcher_handler, 10, NULL, this, 0);

		m_Init =  m_zh != NULL;

		return m_Init;
	}

	void ZKClient::watcher_handler(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
	{
		DEBUG_FUNCTION();

		if(watcherCtx)
		{
			ZKClient* zk = (ZKClient*)watcherCtx;

			try
			{
				zk->m_vWatcherCallback->DoWatcherCallback(zk, type, state, path);
			}
			catch(...)
			{
			}
		}
	}

	void ZKClient::closeZK()
	{
		DEBUG_FUNCTION();

		if(m_zh)
		{
			zookeeper_close(m_zh);
			m_zh = NULL;
		}
	}

	bool ZKClient::getChildren(const char* path, NodeVector& nodes, const bool watch /*= false*/)
	{
		DEBUG_FUNCTION();

		assert(NULL != path);

		struct String_vector sv = {0};

		int code = zoo_get_children(m_zh, path, watch ? 1 : 0, &sv);

		logger->debug(LTRACE, "zoo_get_children [%s] return %d", path, code);

		bool result = (ZOK == code && sv.count > 0);

		if(result)
		{
			for(int i = 0; i < sv.count; i++)
			{
				std::string tmp = sv.data[i];
				if(tmp.length() > 0)
				{
					nodes.push_back(tmp);
				}
			}
		}

		deallocate_String_vector(&sv);

		return result;
	}

	bool ZKClient::getValue(const char* path, std::string& value, const bool watch /*= false*/)
	{
		DEBUG_FUNCTION();

		assert(NULL != path);

		int len = 1024;
		LWPR::Buffer buf(len + 1);

		int code = zoo_get(m_zh, path, watch ? 1 : 0, buf.Inout(), &len, NULL);

		logger->debug(LTRACE, "zoo_get [%s] return %d", path, code);

		if(ZOK == code && len > 0)
		{
			value = buf.Inout();
			return true;
		}

		return false;
	}
}
