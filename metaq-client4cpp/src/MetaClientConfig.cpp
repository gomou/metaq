/*
 * $Id$
 */
#include "MetaClientConfig.h"

namespace META
{
	MetaClientConfig::MetaClientConfig()
	{
	}

	MetaClientConfig::~MetaClientConfig()
	{
	}

	void MetaClientConfig::DoPropConstruct()
	{

	}

	std::string MetaClientConfig::getConfigServerAddress()
	{
		return GetPropertyValue("meta.configServerAddress", "");
	}

	std::string MetaClientConfig::getDiamondZKGroup()
	{
		return GetPropertyValue("meta.diamondZKGroup", "DEFAULT_GROUP");
	}

	std::string MetaClientConfig::getDiamondZKDataId()
	{
		return GetPropertyValue("meta.diamondZKDataId", "metamorphosis.zkConfig");
	}

	int MetaClientConfig::getLogLevel()
	{
		// Ä¬ÈÏDEBUG¼¶±ð
		return GetPropertyValue("meta.logLevel", 5);
	}

	bool MetaClientConfig::isZKEnable()
	{
		return GetPropertyValue("meta.zkEnable", false);
	}

	std::string MetaClientConfig::getZKConnectString()
	{
		return GetPropertyValue("meta.zkConnect", "10.232.102.188:2181,10.232.102.189:2181,10.232.102.190:2181");
	}
}
