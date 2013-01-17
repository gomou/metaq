/*
 * $Id$
 */
#ifndef _LIBMCLI_MESSAGE_SESSION_FACTORY_H__
#define _LIBMCLI_MESSAGE_SESSION_FACTORY_H__

#include <lwpr.h>
#include <list>

#include "Shutdownable.h"
#include "FetchConfigThread.h"
#include "MetaClientConfig.h"
#include "RemotingClientWrapper.h"

namespace META
{
	class MessageProducer;

	class MessageSessionFactory : public LWPR::Object, public virtual Shutdownable
	{
	public:

		MessageSessionFactory() {};

		virtual ~MessageSessionFactory() {};

		/**
		 * 创建消息生产者，默认使用轮询分区选择器
		 */
		virtual MessageProducer* createProducer() = 0;
	};

	class MetaMessageSessionFactory : public MessageSessionFactory
	{
		typedef std::list<MessageProducer*> SessionList;

	public:
		MetaMessageSessionFactory(MetaClientConfig_ptr config = NULL);

		~MetaMessageSessionFactory();

		virtual MessageProducer* createProducer();

		virtual void shutdown();

		void removeChild(MessageProducer* child);

		FetchConfigThread_ptr getFetchConfigThread();

		RemotingClientWrapper_ptr getRemotingClientWrapper();

	private:
		void addChild(MessageProducer* child);

	private:
		LWPR::RecursiveMutex m_Mutex;
		SessionList m_SessionList;

		MetaClientConfig_var m_vMetaClientConfig;
		FetchConfigThread_var m_vFetchConfigThread;
		RemotingClientWrapper_var m_vRemotingClientWrapper;
	};

	DECLAREVAR(MessageSessionFactory);
	DECLAREVAR(MetaMessageSessionFactory);
}
#endif // end of _LIBMCLI_MESSAGE_SESSION_FACTORY_H__
