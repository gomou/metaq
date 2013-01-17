/*
 * $Id$
 */
#ifndef _LIBMCLI_REMOTING_CLIENT_WRAPPER_H__
#define _LIBMCLI_REMOTING_CLIENT_WRAPPER_H__

#include <lwpr.h>
#include <map>

#include "Message.h"

namespace META
{
	typedef std::map<std::string, LWPR::SOCKET_FD_T> AddressSocketMap;

	typedef struct
	{
		LWPR::INT64 msgId;
		LWPR::INT32 partitionId;
		LWPR::INT64 repCode;
	} MetaResponseData;

	class RemotingClientWrapper : public LWPR::Object
	{
	public:
		RemotingClientWrapper();

		~RemotingClientWrapper();

		LWPR::SOCKET_INVOKE_RET_E invokeToGroup(const char* address,
		                                      const char* requestData,
		                                      LWPR::INT32 size,
		                                      LWPR::UINT32 opaque,
		                                      LWPR::INT32 timeout,
		                                      MetaResponseData& responseData);

	private:
		LWPR::SOCKET_FD_T GetAndCreateConnection(const char* address);

		void CloseConnection(const char* address);

	private:
		LWPR::Mutex m_Mutex;
		AddressSocketMap m_AddressSocketMap;
	};

	DECLAREVAR(RemotingClientWrapper);
}
#endif // end of _LIBMCLI_REMOTING_CLIENT_WRAPPER_H__
