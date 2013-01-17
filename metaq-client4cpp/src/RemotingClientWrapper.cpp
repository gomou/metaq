/*
 * $Id$
 */
#include "RemotingClientWrapper.h"

namespace META
{
	RemotingClientWrapper::RemotingClientWrapper()
	{
		DEBUG_FUNCTION();
	}

	RemotingClientWrapper::~RemotingClientWrapper()
	{
		DEBUG_FUNCTION();

		for(AddressSocketMap::iterator it = m_AddressSocketMap.begin();
		    it != m_AddressSocketMap.end(); it++)
		{
			logger->info(LTRACE, "断开与META服务器[%s]的网络连接", it->first.c_str());
			LWPR::Socket::CloseSocket(it->second);
		}

		m_AddressSocketMap.clear();
	}


	LWPR::SOCKET_INVOKE_RET_E RemotingClientWrapper::invokeToGroup(const char* address, const char* requestData, LWPR::INT32 size, LWPR::UINT32 opaque, LWPR::INT32 timeout, MetaResponseData& responseData)
	{
		DEBUG_FUNCTION();

		assert(NULL != address);
		assert(NULL != requestData);

		LWPR::Buffer_var bufResponse = new LWPR::Buffer(1024);

		LWPR::Synchronized syn(m_Mutex);

		LWPR::SOCKET_FD_T fd = GetAndCreateConnection(address);
		if(fd == -1)
		{
			logger->error(LTRACE, "连接服务器[%s]失败", address);
			return LWPR::SOCKET_INVOKE_CONNECT_FAILED;
		}

		LWPR::SOCKET_RET_TYPE_E result = LWPR::Socket::WriteSocket(fd, requestData, size, timeout);
		switch(result)
		{
		case LWPR::SOCKET_RET_CONNECT_FAILED:
		case LWPR::SOCKET_RET_FAILED:
		case LWPR::SOCKET_RET_FREE:
		case LWPR::SOCKET_RET_NOT_WRABLE:
			logger->error(LTRACE, "发送请求数据失败[%s]", address);
			CloseConnection(address);
			return LWPR::SOCKET_INVOKE_SEND_DATA_FAILED;
		case LWPR::SOCKET_RET_TIMEOUT:
			logger->error(LTRACE, "发送请求数据超时[%s]", address);
			CloseConnection(address);
			return LWPR::SOCKET_INVOKE_SEND_DATA_TIMEOUT;
		case LWPR::SOCKET_RET_OK:
		default:
			break;
		}

		while(result == LWPR::SOCKET_RET_OK)
		{
			result = LWPR::Socket::ReadSocketAsPossible(fd, *bufResponse, timeout);
			if(result == LWPR::SOCKET_RET_OK)
			{
				std::string tmp = bufResponse->Inout();
				std::string::size_type pos = tmp.find("\r\n", 0);
				if(std::string::npos != pos)
				{
					// 解析应答头
					memset(bufResponse->Inout() + pos, 0, 2);

					logger->debug(LTRACE, "response header = [%s]", bufResponse->Inout());

					char headerItem[4][32];
					memset(headerItem, 0, sizeof(headerItem));
					int ret = sscanf(bufResponse->Inout(), "%s %s %s %s", headerItem[0]
					                 , headerItem[1]
					                 , headerItem[2]
					                 , headerItem[3]);
					if(4 != ret)
					{
						logger->error(LTRACE, "解析应答报文头出错 sscanf return %d", ret);
						CloseConnection(address);
						return LWPR::SOCKET_INVOKE_POST_PROCESSING_FAILED;
					}

					responseData.repCode = atol(headerItem[1]);
					LWPR::INT64 length = atol(headerItem[2]);
					LWPR::INT64 repOpaque = atol(headerItem[3]);

					// 校验opaque协议的序列号
					if(opaque != (LWPR::UINT32)repOpaque)
					{
						logger->error(LTRACE, "应答协议序列号与请求协议序列号不同 request opaque = [%u] response opaque = [%u]"
						              , opaque, (LWPR::UINT32)repOpaque);
						CloseConnection(address);
						return LWPR::SOCKET_INVOKE_POST_PROCESSING_FAILED;
					}

					// 如果应答报文没有收完整，则继续接收
					LWPR::INT32 remainDataSize = pos + 2 + length - bufResponse->Size();
					if(remainDataSize > 0)
					{
						result = LWPR::Socket::ReadSocket(fd, bufResponse->Inout() + bufResponse->Size()
						                                  , remainDataSize, timeout);
						if(result != LWPR::SOCKET_RET_OK)
						{
							logger->error(LTRACE, "读剩余应答报文体错误 ReadSocket return %d", result);
							CloseConnection(address);
							return LWPR::SOCKET_INVOKE_RECEIVE_DATA_FAILED;
						}
					}

					// 解析应答体（因为编码是UTF-8，这个字符集与ANSII是兼容的）
					char bodyItem[3][32];
					memset(bodyItem, 0, sizeof(bodyItem));
					ret = sscanf(bufResponse->Inout() + pos + 2, "%s %s %s"
					             , bodyItem[0]
					             , bodyItem[1]
					             , bodyItem[2]);
					if(3 != ret)
					{
						logger->error(LTRACE, "解析应答包头体出错 sscanf return %d", ret);
						CloseConnection(address);
						return LWPR::SOCKET_INVOKE_POST_PROCESSING_FAILED;
					}

					logger->debug(LTRACE, "response body = [%s]", bufResponse->Inout() + pos + 2);

					responseData.msgId = atol(bodyItem[0]);
					responseData.partitionId = atoi(bodyItem[1]);

					return LWPR::SOCKET_INVOKE_OK;
				}
				else
				{
					continue;
				}
			}
		}

		return LWPR::SOCKET_INVOKE_RECEIVE_DATA_FAILED;
	}

	LWPR::SOCKET_FD_T RemotingClientWrapper::GetAndCreateConnection(const char* address)
	{
		DEBUG_FUNCTION();

		assert(NULL != address);

		AddressSocketMap::iterator it = m_AddressSocketMap.find(address);
		if(m_AddressSocketMap.end() != it) return it->second;

		LWPR::SOCKET_FD_T fd = LWPR::Socket::ConnectRemoteHost(address);
		if(fd != LWPR::SOCKET_RET_FAILED)
		{
			logger->info(LTRACE, "建立与META服务器[%s]的网络连接", address);
			m_AddressSocketMap[address] = fd;
		}

		return fd;
	}

	void RemotingClientWrapper::CloseConnection(const char* address)
	{
		DEBUG_FUNCTION();

		assert(NULL != address);

		AddressSocketMap::iterator it = m_AddressSocketMap.find(address);
		if(m_AddressSocketMap.end() != it)
		{
			LWPR::Socket::CloseSocket(it->second);
			m_AddressSocketMap.erase(it);
		}
	}
}
