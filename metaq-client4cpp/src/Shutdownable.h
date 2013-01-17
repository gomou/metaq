/*
 * $Id$
 */
#ifndef _LIBMCLI_SHUTDOWNABLE_H__
#define _LIBMCLI_SHUTDOWNABLE_H__

namespace META
{
	class Shutdownable
	{
	public:

		Shutdownable();
		virtual ~Shutdownable();

		bool hasShutdown();

		virtual void shutdown();

	private:
		volatile bool m_shutdown;
	};
}
#endif // end of _LIBMCLI_SHUTDOWNABLE_H__
