/*
 * $Id: PmonCmdline.h 3 2011-08-19 02:25:45Z  $
 */
#ifndef TUX_PMON_CMDLINE_H__
#define TUX_PMON_CMDLINE_H__

#include <lwpr.h>

class PmonCmdline : public LWPR::ProgramOption
{
	PmonCmdline();

public:
	static PmonCmdline* GetInstance();

	/**
	 * [-d logdir]
	 */
	std::string GetLogDir();

	/**
	 * [-i statistics interval ]
	 */
	int GetStatisticsInterval();

	/**
	 * [pid . . . ]
	 */
	void GetPIDS(LWPR::PIDList& pids);

private:
	bool CheckCmdline();

};

#endif // end of TUX_VIEWC32_CMDLINE_H__
