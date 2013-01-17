/*
 * $Id: Mutex.cpp 3 2011-08-19 02:25:45Z  $
 */
#include "Mutex.h"
#include "Thread.h"

namespace LWPR
{

	Mutex::Mutex()
	{
		int retcode = 0;

		retcode = pthread_mutex_init(&m_mutex, NULL);
		if(retcode != 0)
			throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutex_init error");
	}

	Mutex::~Mutex()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	void Mutex::Lock()
	{
		int retcode = 0;

		retcode = pthread_mutex_lock(&m_mutex);
		if(retcode != 0)
			throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutex_lock error");
	}

	bool Mutex::TryLock()
	{
		int retcode = 0;

		retcode = pthread_mutex_trylock(&m_mutex);

		return (retcode == 0);
	}

	void Mutex::Unlock()
	{
		int retcode = 0;

		retcode = pthread_mutex_unlock(&m_mutex);
		//if(retcode != 0)
		//	throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutex_unlock error");
	}

	PTHREAD_MUTEX_T* Mutex::GetMutexRef()
	{
		return &m_mutex;
	}

	//--------------------------------------------------------------------------
	// class RecursiveMutex
	//--------------------------------------------------------------------------
	RecursiveMutex::RecursiveMutex()
	{
		int retcode = 0;

		retcode = pthread_mutexattr_init(&m_mutexattr);
		if(retcode != 0)
			throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutexattr_init error");

		retcode = pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
		if(retcode != 0)
		{
			pthread_mutexattr_destroy(&m_mutexattr);
			throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutexattr_settype error");
		}

		retcode = pthread_mutex_init(&m_mutex, &m_mutexattr);
		if(retcode != 0)
		{
			pthread_mutexattr_destroy(&m_mutexattr);
			throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutex_init error");
		}
	}

	RecursiveMutex::~RecursiveMutex()
	{
		pthread_mutexattr_destroy(&m_mutexattr);
		pthread_mutex_destroy(&m_mutex);
	}

	void RecursiveMutex::Lock()
	{
		int retcode = 0;

		retcode = pthread_mutex_lock(&m_mutex);
		if(retcode != 0)
			throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutex_lock error");
	}

	bool RecursiveMutex::TryLock()
	{
		int retcode = 0;

		retcode = pthread_mutex_trylock(&m_mutex);

		return (retcode == 0);
	}

	void RecursiveMutex::Unlock()
	{
		int retcode = 0;

		retcode = pthread_mutex_unlock(&m_mutex);
		//if(retcode != 0)
		//	throw LWPR::LWPR_THREAD_API_ERR(EXCEPTION_THREAD_TRACE, retcode, "pthread_mutex_unlock error");
	}

	PTHREAD_MUTEX_T* RecursiveMutex::GetMutexRef()
	{
		return &m_mutex;
	}

};
