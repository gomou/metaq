/*
 * $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <lwpr.h>

class FileOffsetPersist
{
	

public:
	FileOffsetPersist()
	{
	}

	~FileOffsetPersist()
	{

	}

	bool init(const char* file)
	{
	
	}
};

int main(int argc, char **argv)
{
	char bufCommand[512] = {0};

	sprintf(bufCommand, "tail --bytes=10000000 -f %s", argv[1]);

	FILE *fp = popen(bufCommand, "r");
	if(fp != NULL)
	{
		unsigned long long index = 0;

		while(1)
		{
			char bufLine[1024 + 1] = {0};

			while(fgets(bufLine, 1024, fp))
			{
				printf("%d %s", ++index, bufLine);
			}

			sleep(1);
		}

		pclose(fp);
	}

	return -1;
}
