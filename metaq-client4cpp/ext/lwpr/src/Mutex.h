/*
 * $Id: Mutex.h 3 2011-08-19 02:25:45Z  $
 */
#ifndef LWPR_MUTEX_H__
#define LWPR_MUTEX_H__
#include "LWPRType.h"
#include "Exception.h"

namespace LWPR
{

	typedef pthread_mutex_t  PTHREAD_MUTEX_T;
	typedef pthread_mutexattr_t  PTHREAD_MUTEXATTR_T;

	class Mutex
	{
	public:

		/**
		 * 构造函数
		 */
		Mutex();

		/**
		 * 析构函数
		 */
		~Mutex();

		/**
		 * 加锁
		 */
		void Lock();

		/**
		 * 非阻塞加锁
		 * 如果锁定成功返回true，否则返回false
		 */
		bool TryLock();

		/**
		 * 释放锁
		 */
		void Unlock();

		/**
		 * 获取锁，为条件变量准备
		 */
		PTHREAD_MUTEX_T* GetMutexRef();

	private:

		PTHREAD_MUTEX_T m_mutex;
	};

	class RecursiveMutex
	{
	public:

		/**
		 * 构造函数
		 */
		RecursiveMutex();

		/**
		 * 析构函数
		 */
		~RecursiveMutex();

		/**
		 * 加锁
		 */
		void Lock();

		/**
		 * 非阻塞加锁
		 * 如果锁定成功返回true，否则返回false
		 */
		bool TryLock();

		/**
		 * 释放锁
		 */
		void Unlock();

		/**
		 * 获取锁，为条件变量准备
		 */
		PTHREAD_MUTEX_T* GetMutexRef();

	private:
		PTHREAD_MUTEXATTR_T m_mutexattr;
		PTHREAD_MUTEX_T m_mutex;
	};
};

#endif // end of LWPR_MUTEX_H__
