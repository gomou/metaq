/*
 * $Id: test1.cpp 13 2011-08-25 07:30:20Z  $
 */
#include <iostream>
#include <stdio.h>
#include <signal.h>

#include <NotifyUtil.h>
#include <lwpr.h>

#include <unistd.h>

#include <string>

using std::cout;
using std::cerr;
using std::endl;

static int run(int argc, char** argv)
{
	char url[1024] = {0};

	if(argc != 3)
	{
		printf("Useage: %s dataId group\n", argv[0]);
		return 0;
	}

	sprintf(url, "http://10.232.12.32:8080/diamond-server/config.co?dataId=%s&group=%s", argv[1], argv[2]);

	NOTIFY::NotifyUtil::HTTPReqHeader reqheader;

	while(1)
	{
		NOTIFY::NotifyUtil::HTTPRepHeader repheader;
		std::string http_content;
		bool result = NOTIFY::NotifyUtil::PerformHttpGetRequest(
		                  url,
		                  reqheader,
		                  repheader,
		                  http_content);



		printf("PerformHttpGetRequest = %s\n", result ? "OK" : "Failed");
		printf("HTTPVERSION = %s\n", repheader.version.c_str());
		printf("HTTPRETCODE = %d\n", repheader.code);
		printf("HTTPRETDESC = %s\n", repheader.desc.c_str());

		for(NOTIFY::NotifyUtil::HTTPReqHeader::iterator it = repheader.items.begin();
		    it != repheader.items.end();
		    it++)
		{
			printf("\t[%s] = [%s]\n", it->first.c_str(), it->second.c_str());
		}

		printf("[%s]\n", http_content.c_str());

		reqheader["Content-MD5"] = repheader.items["Content-MD5"];

		sleep(1);
	}
	return 0;
}

int main(int argc, char** argv)
{
	try
	{
		return run(argc, argv);
	}
	catch(LWPR::Exception& e)
	{
		cerr << e.what() << endl;
	}
	catch(std::exception& e)
	{
		cerr << e.what() << endl;
	}
	catch(...)
	{
		cerr << "Unknow exception" << endl;
	}

	return -1;
}
