#include <stdio.h>
#include <META/MessageSessionFactory.h>
#include <META/MessageProducer.h>

char bufMessage[1024 * 1024];

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Useage: %s count\n", argv[0]);
		return -1;
	}

	META::MessageSessionFactory_var sessionFactory = new META::MetaMessageSessionFactory();

	META::MessageProducer_var producer = sessionFactory->createProducer();

	const char* topic = "meta-test";

	producer->publish(topic);

	memset(bufMessage, 'M', sizeof(bufMessage));

	for(int i = 0; i < atoi(argv[1]); i++)
	{
		META::Message msg(topic, bufMessage, sizeof(bufMessage));

		META::SendResult sendResult = producer->sendMessage(msg);
		if (!sendResult.isSuccess()) {
			printf("Send message failed,error message: %d %s\n", sendResult.getErrorCode(), sendResult.getErrorMessage().c_str());
		}
		else {
			printf("Send message successfully %d\n", i);
		}
	}

	producer->shutdown();

	sessionFactory->shutdown();

	return 0;
}
