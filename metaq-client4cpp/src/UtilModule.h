/*
 * $Id: NotifyUtil.h 13 2011-08-25 07:30:20Z  $
 */
#ifndef _LIBMCLI_NOTIFY_UTIL_H__
#define _LIBMCLI_NOTIFY_UTIL_H__

#include <string>
#include <map>
#include <lwpr.h>

namespace Util
{
	typedef std::map<std::string, std::string> NameValuePair;

	typedef NameValuePair HTTPReqHeader;

	typedef struct
	{
		std::string version;
		int code;
		std::string desc;
		NameValuePair items;
	} HTTPRepHeader;

	void ConvertHttpHeader(const std::string& str_repheader, HTTPRepHeader& struct_repheader);

	bool PerformHttpGetRequest(const char* url,
	                           const HTTPReqHeader& reqheader,
	                           const int timeout, // 超时时间， 单位秒
	                           HTTPRepHeader& repheader,
	                           std::string& repcontent);

	bool PerformHttpPostRequest(const char* url,
	                            const HTTPReqHeader& reqheader,
	                            const char* postfileds,
	                            const int timeout,
	                            HTTPRepHeader& repheader,
	                            std::string& repcontent);

	bool PerformHttpPostRequest(const char* url,
	                            const HTTPReqHeader& reqheader,
	                            const NameValuePair& postfileds,
	                            const int timeout,
	                            HTTPRepHeader& repheader,
	                            std::string& repcontent);

	bool LoadFileToBuffer(const char* file, LWPR::Buffer& buf);

	bool HttpDecode(const char* url, int length, std::string& out);

	bool HttpEncode(const char* url, int length, std::string& out);
}
#endif // end of _LIBMCLI_NOTIFY_UTIL_H__
