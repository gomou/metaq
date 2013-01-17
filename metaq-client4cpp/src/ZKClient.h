/*
 * $Id$
 */
#ifndef _LIBMCLI_ZKCLIENT_H__
#define _LIBMCLI_ZKCLIENT_H__

#include <lwpr.h>
#include <ZK/zookeeper.h>

namespace META
{
	class ZKClient;

	typedef std::vector<std::string> NodeVector;

	class WatcherCallback : virtual public LWPR::Object
	{
	public:
		virtual ~WatcherCallback() {};

		virtual void DoWatcherCallback(ZKClient* zk, int type, int state, const char *path) = 0;
	};

	DECLAREVAR(WatcherCallback);

	class ZKClient : public LWPR::Object
	{
	public:
		ZKClient(const char* hosts, WatcherCallback_ptr watcher = NULL);
		~ZKClient();

		bool init();

		void closeZK();

		bool getChildren(const char* path, NodeVector& nodes, const bool watch = false);

		bool getValue(const char* path, std::string& value, const bool watch = false);
		
		static void watcher_handler(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

	private:
		std::string m_hosts;
		WatcherCallback_var m_vWatcherCallback;
		zhandle_t *m_zh;
		volatile bool m_Init;
	};

	DECLAREVAR(ZKClient);
}
#endif // end of _LIBMCLI_ZKCLIENT_H__
