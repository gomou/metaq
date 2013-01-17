/*
 * $Id$
 */
#include "Message.h"

namespace META
{
	Message::Message(const char* topic, void* data, int size, const char* attribute /*= ""*/)
		: m_topic(topic)
		, m_data(data, size)
		, m_attribute(attribute)
		, m_partition(-1)
	{
	}

	Message::~Message()
	{
	}

	LWPR::INT64 Message::getId() const
	{
		return m_id;
	}

	void Message::setId(LWPR::INT64 id)
	{
		m_id = id;
	}

	std::string Message::getTopic() const
	{
		return m_topic;
	}

	std::string Message::getAttribute() const
	{
		return m_attribute;
	}

	LWPR::Buffer& Message::getData()
	{
		return m_data;
	}

	LWPR::INT32 Message::getFlag() const
	{
		return m_flag;
	}

	LWPR::INT32 Message::getPartition() const
	{
		return m_partition;
	}

	void Message::setPartition(LWPR::INT32 id)
	{
		m_partition = id;
	}

	void Message::encode(LWPR::INT32 partition, LWPR::UINT32 opaque, LWPR::Buffer& marshalData)
	{
		marshalData.Reset();
		marshalData.Capacity(512);

		sprintf(marshalData.Inout(), "put %s %d %d %d %u\r\n"
		        , m_topic.c_str()
		        , partition
		        , m_data.Size()
		        , m_flag
		        , opaque);

		LWPR::INT32 headerLength = strlen(marshalData.Inout());

		marshalData.Size(headerLength + m_data.Size());

		memcpy(marshalData.Inout() + headerLength, m_data.Inout(), m_data.Size());
	}
}
