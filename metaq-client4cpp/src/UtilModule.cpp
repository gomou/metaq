/*
 * $Id: NotifyUtil.cpp 13 2011-08-25 07:30:20Z  $
 */
#include "UtilModule.h"
#include <curl/curl.h>

#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

#include <lwpr.h>

namespace Util
{
	static size_t WriteResponseCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
	{
		if(userdata == NULL)
			return 0;

		((std::string*)userdata)->append(ptr, size * nmemb);

		return size * nmemb;
	}


	bool PerformHttpGetRequest(const char* url, const HTTPReqHeader& reqheader, const int timeout, HTTPRepHeader& repheader, std::string& repcontent)
	{
		assert(NULL != url);

		CURL *curl;
		CURLcode res;
		struct curl_slist *chunk = NULL;
		std::string strheader;

		curl = curl_easy_init();
		if(!curl) goto END;

		res = curl_easy_setopt(curl, CURLOPT_URL, url);
		if(res != CURLE_OK) goto END;

		if(!reqheader.empty())
		{
			for(HTTPReqHeader::const_iterator it = reqheader.begin();
			    it != reqheader.end();
			    it++)
			{
				chunk = curl_slist_append(chunk, (it->first + ": " + it->second).c_str());
			}

			res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
			if(res != CURLE_OK) goto END;
		}

		//
		//	Pass a long as parameter containing the maximum time in seconds that you allow the libcurl transfer
		//	operation to take. Normally, name lookups can take a considerable time and limiting operations
		//	to less than a few minutes risk aborting perfectly normal operations. This option will cause curl to
		//	use the SIGALRM to enable time-outing system calls.
		//	In unix-like systems, this might cause signals to be used unless CURLOPT_NOSIGNAL is set.
		//
		res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
		if(res != CURLE_OK) goto END;

		res = curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &strheader);
		if(res != CURLE_OK) goto END;

		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &repcontent);
		if(res != CURLE_OK) goto END;

		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteResponseCallback);
		if(res != CURLE_OK) goto END;

		res = curl_easy_perform(curl);

END:
		if(curl) curl_easy_cleanup(curl);

		if(chunk) curl_slist_free_all(chunk);

		if(res == CURLE_OK)
		{
			ConvertHttpHeader(strheader, repheader);
			return true;
		}

		return false;
	}

	void ConvertHttpHeader(const std::string& str_repheader, HTTPRepHeader& struct_repheader)
	{
		std::vector<std::string> items;

		for(std::string::size_type index = 0;;)
		{
			std::string::size_type pos = str_repheader.find("\r\n", index);
			if(std::string::npos != pos)
			{
				int len = pos - index;

				if(len > 0)
					items.push_back(str_repheader.substr(index, len));

				index = pos + 2;
			}
			else
			{
				break;
			}
		}

		if(!items.empty())
		{
			char buf[3][128];
			memset(buf, 0, sizeof(buf));

			sscanf(items[0].c_str(), "%s %s %[^$]", buf[0], buf[1], buf[2]);

			struct_repheader.version = buf[0];
			struct_repheader.code = atoi(buf[1]);
			struct_repheader.desc = buf[2];
		}

		for(size_t i = 1; i < items.size(); i++)
		{
			char buf1[64] = {0};
			char buf2[1024] = {0};

			sscanf(items[i].c_str(), "%[^ :]%*[ :]%[^$]", buf1, buf2);

			LWPR::StringUtil::TrimAll(buf2);
			struct_repheader.items[buf1] = buf2;
		}
	}

	bool PerformHttpPostRequest(const char* url, const HTTPReqHeader& reqheader, const char* postfileds, const int timeout, HTTPRepHeader& repheader, std::string& repcontent)
	{
		assert(NULL != url);
		assert(NULL != postfileds);

		CURL *curl;
		CURLcode res;
		struct curl_slist *chunk = NULL;
		std::string strheader;

		curl = curl_easy_init();
		if(!curl) goto END;

		res = curl_easy_setopt(curl, CURLOPT_URL, url);
		if(res != CURLE_OK) goto END;

		if(!reqheader.empty())
		{
			for(HTTPReqHeader::const_iterator it = reqheader.begin();
			    it != reqheader.end();
			    it++)
			{
				chunk = curl_slist_append(chunk, (it->first + ": " + it->second).c_str());
			}

			res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
			if(res != CURLE_OK) goto END;
		}

		res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
		if(res != CURLE_OK) goto END;

		res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfileds);
		if(res != CURLE_OK) goto END;

		res = curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &strheader);
		if(res != CURLE_OK) goto END;

		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &repcontent);
		if(res != CURLE_OK) goto END;

		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteResponseCallback);
		if(res != CURLE_OK) goto END;

		res = curl_easy_perform(curl);

END:
		if(curl) curl_easy_cleanup(curl);

		if(chunk) curl_slist_free_all(chunk);

		if(res == CURLE_OK)
		{
			ConvertHttpHeader(strheader, repheader);
			return true;
		}

		return false;
	}

	bool PerformHttpPostRequest(const char* url, const HTTPReqHeader& reqheader, const NameValuePair& postfileds, const int timeout, HTTPRepHeader& repheader, std::string& repcontent)
	{
		std::string strfileds;

		NameValuePair::const_iterator it = postfileds.begin();
		for(; it != postfileds.end(); it++)
		{
			strfileds += it->first;
			strfileds += "=";
			strfileds += it->second;
			strfileds += "&";
		}

		return PerformHttpPostRequest(url, reqheader, strfileds.c_str(), timeout, repheader, repcontent);
	}

	bool LoadFileToBuffer(const char* file, LWPR::Buffer& buf)
	{
		if(!file) return false;

		FILE* fp = fopen(file, "rb");
		if(fp)
		{
			struct stat bufStat = {0};
			if(fstat(fileno(fp), &bufStat) != -1)
			{
				buf.Reset();
				buf.Size(bufStat.st_size);

				if(fread(buf.Inout(), bufStat.st_size, 1, fp) != 1)
				{
					fclose(fp);
					return false;
				}
				else
				{
					fclose(fp);
					return true;
				}
			}

			fclose(fp);
		}

		return false;
	}

	bool HttpDecode(const char* url, int length, std::string& out)
	{
		if(url)
		{
			char* pRet = curl_unescape(url , length);
			if(pRet)
			{
				out = pRet;
				curl_free(pRet);
				return true;
			}
		}

		return false;
	}

	bool HttpEncode(const char* url, int length, std::string& out)
	{
		if(url)
		{
			char* pRet = curl_escape(url , length);
			if(pRet)
			{
				out = pRet;
				curl_free(pRet);
				return true;
			}
		}

		return false;
	}

}
