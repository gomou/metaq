/*
 * $Id$
 */
#ifndef _LIBMCLI_MESSAGE_H__
#define _LIBMCLI_MESSAGE_H__

#include <lwpr.h>

namespace META
{
	class Message
	{
	public:
		Message(const char* topic, void* data, int size, const char* attribute = "");

		virtual ~Message();

		LWPR::INT64 getId() const;

		void setId(LWPR::INT64 id);

		std::string getTopic() const;

		std::string getAttribute() const;

		LWPR::Buffer& getData();

		LWPR::INT32 getFlag() const;

		LWPR::INT32 getPartition() const;

		void setPartition(LWPR::INT32 id);

		void encode(LWPR::INT32 partition, LWPR::UINT32 opaque, LWPR::Buffer& marshalData);

	private:
		LWPR::INT64 m_id;
		std::string m_topic;
		LWPR::Buffer m_data;
		std::string m_attribute;
		LWPR::INT32 m_flag;
		LWPR::INT32 m_partition;
	};
}
#endif // end of _LIBMCLI_MESSAGE_H__
