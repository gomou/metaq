/*
 * $Id: Synchronized.cpp 3 2011-08-19 02:25:45Z  $
 */
#include "Synchronized.h"

namespace LWPR
{
	Synchronized::Synchronized(const IPCID_T id)
		: m_synType(SYN_IPCSEM),
		  m_id(id),
		  m_pMutex(NULL),
		  m_pRecursiveMutex(NULL),
		  m_pSemph(NULL)
	{
		IPCSEM::Lock(m_id, true, false);
	}

	Synchronized::Synchronized(Mutex &lock)
		: m_synType(SYN_MUTEX),
		  m_id(IPC_INVALID_ID),
		  m_pMutex(&lock),
		  m_pRecursiveMutex(NULL),
		  m_pSemph(NULL)
	{
		lock.Lock();
	}

	Synchronized::Synchronized(RecursiveMutex &lock)
		: m_synType(SYN_RECURSIVEMUTEX),
		  m_id(IPC_INVALID_ID),
		  m_pMutex(NULL),
		  m_pRecursiveMutex(&lock),
		  m_pSemph(NULL)
	{
		lock.Lock();
	}

	Synchronized::Synchronized(Semph &lock)
		: m_synType(SYN_SEMPH),
		  m_id(IPC_INVALID_ID),
		  m_pMutex(NULL),
		  m_pRecursiveMutex(NULL),
		  m_pSemph(&lock)
	{
		lock.Lock();
	}

	Synchronized::~Synchronized()
	{
		switch(m_synType)
		{
		case SYN_IPCSEM:
			IPCSEM::Unlock(m_id, false);
			break;
		case SYN_RECURSIVEMUTEX:
			m_pRecursiveMutex->Unlock();
			break;
		case SYN_MUTEX:
			m_pMutex->Unlock();
			break;
		case SYN_SEMPH:
			m_pSemph->Unlock();
			break;
		default:
			assert(0);
		}
	}

	//----------------------------------------------------------------------------------
	// class SynchronizedRead
	//----------------------------------------------------------------------------------
	SynchronizedRead::SynchronizedRead(RWMutex &lock)
		: m_pRWMutex(&lock)
	{
		m_pRWMutex->RDLock();
	}

	SynchronizedRead::~SynchronizedRead()
	{
		m_pRWMutex->Unlock();
	}

	//----------------------------------------------------------------------------------
	// class SynchronizedWrite
	//----------------------------------------------------------------------------------
	SynchronizedWrite::SynchronizedWrite(RWMutex &lock)
		: m_pRWMutex(&lock)
	{
		m_pRWMutex->WRLock();
	}

	SynchronizedWrite::~SynchronizedWrite()
	{
		m_pRWMutex->Unlock();
	}
};
