/*
 * $Id$
 */
#ifndef _LIBMCLI_META_CLIENT_CONFIG_H__
#define _LIBMCLI_META_CLIENT_CONFIG_H__

#include <lwpr.h>
#include <string>

namespace META
{
	class MetaClientConfig : public LWPR::Object, public LWPR::ConfigProperty
	{
	public:
		MetaClientConfig();
		~MetaClientConfig();

		virtual void DoPropConstruct();

		std::string getConfigServerAddress();

		std::string getDiamondZKGroup();

		std::string getDiamondZKDataId();

		bool isZKEnable();

		std::string getZKConnectString();

		/*
		const int LOGGER_LEVEL_NONE = 0;		// 不打印日志
		const int LOGGER_LEVEL_FATAL = 1;
		const int LOGGER_LEVEL_ERROR = 2;
		const int LOGGER_LEVEL_WARN = 3;
		const int LOGGER_LEVEL_INFO = 4;
		const int LOGGER_LEVEL_DEBUG = 5;
		const int LOGGER_LEVEL_TRACE = 6;
		const int LOGGER_LEVEL_MAX = 7;
		*/
		int getLogLevel();

	private:
	};

	DECLAREVAR(MetaClientConfig);
}
#endif // end of _LIBMCLI_META_CLIENT_CONFIG_H__
