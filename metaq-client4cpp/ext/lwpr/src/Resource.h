/*
 * $Id: Resource.h 3 2011-08-19 02:25:45Z  $
 */
#ifndef LWPR_RESOURCE_H__
#define LWPR_RESOURCE_H__
#include "LWPRType.h"
#include "Exception.h"
#include "Mutex.h"

namespace LWPR
{
	typedef pthread_cond_t PTHREAD_COND_T;

	class Resource
	{
	public:
		Resource();

		~Resource();

		void Wait();

		void Wait(int seconds);

		void Notify();

		void Notifyall();

	private:
		Mutex m_mutex;
		PTHREAD_COND_T m_cond;
		volatile int m_nWaitThreads;
		volatile int m_nResource;
	};
};

#endif // end of LWPR_RESOURCE_H__
