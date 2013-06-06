/**
 *	@file NMEErrorCpp.h
 *	@author Yves Piguet.
 *	@copyright 2007-2011, Yves Piguet.
 *	@brief C++ class for NME exceptions.
 */

/* License: new BSD license (see NME.h) */

#if !defined(__NMEErrorCpp__) && defined(UseNMECppException)
#define __NMEErrorCpp__

#include "NME.h"

/** NME error class, used by NME::getOutput */
class NMEError
{
	public:
		NMEErr err;	///< error code
		
		/** Constructor
		@param[in] err error code
		*/
		NMEError(NMEErr err)
		{
			this->err = err;
		}
		
		/** Get error code
		@return error code
		*/
		NMEErr code()
		{
			return err;
		}
};

#endif
