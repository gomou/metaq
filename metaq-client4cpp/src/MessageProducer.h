/*
 * $Id$
 */
#ifndef _LIBMCLI_MESSAGE_PRODUCER_H__
#define _LIBMCLI_MESSAGE_PRODUCER_H__

#include <lwpr.h>
#include "Message.h"
#include "MessageSessionFactory.h"

namespace META
{
	class SendResult
	{
		bool m_success;
		int m_errcode;
		std::string m_errorMessage;

	public:

		SendResult(bool success, const char* errorMessage);

		bool isSuccess();

		void enableSuccess();

		void enableFailed(int code, const char* errorMessage);

		int getErrorCode();

		std::string getErrorMessage();
	};

	class MessageProducer : public LWPR::Object, virtual public Shutdownable
	{
	public:

		MessageProducer() {};

		virtual ~MessageProducer() {};

		/**
		 * 发布topic，以便producer从zookeeper获取broker列表并连接，在发送消息前必须先调用此方法
		 */
		virtual void publish(const char* topic) = 0;

		/**
		 * 发送消息
		 */
		virtual SendResult sendMessage(Message& message) = 0;

		/**
		 * 发送消息
		 */
		virtual SendResult sendMessage(Message* message) = 0;
	};

	DECLAREVAR(MessageProducer);

	class SimpleMessageProducer : public MessageProducer
	{
	public:

		SimpleMessageProducer(MetaMessageSessionFactory* factory);
		~SimpleMessageProducer();

		virtual void publish(const char* topic);

		virtual SendResult sendMessage(Message& message);

		virtual SendResult sendMessage(Message* message);

		virtual void shutdown();

	private:
		MetaMessageSessionFactory_var m_vFactory;
		FetchConfigThread_var m_vFetchConfigThread;
		RemotingClientWrapper_var m_vRemotingClientWrapper;
		LWPR::AtomicInteger m_nSelectFactor;
		static int MAX_RETRY;
	};
}
#endif // end of _LIBMCLI_MESSAGE_PRODUCER_H__
