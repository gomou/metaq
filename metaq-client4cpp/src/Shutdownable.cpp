/*
 * $Id$
 */
#include "Shutdownable.h"

namespace META
{
	Shutdownable::Shutdownable() : m_shutdown(false)
	{
	}

	Shutdownable::~Shutdownable()
	{
	}

	bool Shutdownable::hasShutdown()
	{
		return m_shutdown;
	}

	void Shutdownable::shutdown()
	{
		m_shutdown = true;
	}

}
