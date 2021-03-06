/**
 *	@file NMEPluginUppercase.h
 *	@brief NME optional plugin for cnverting text to uppercase.
 *	@author Yves Piguet.
 *	@copyright 2007-2011, Yves Piguet.
 */
 
/* License: new BSD license (see NME.h) */

#ifndef __NMEPluginUppercase__
#define __NMEPluginUppercase__

#ifdef __cplusplus
extern "C" {
#endif

#include "NME.h"

/** Plugin implementation for reversing text
	@param[in] name plugin name, such as "reverse"
	@param[in] nameLen length of name
	@param[in] data data text
	@param[in] dataLen length of data
	@param[in,out] context current context
	@param[in] userData pointer passed from the parser, as specified in NMEPlugin
	@return error code (kNMEErrOk for success)
	@test @code
	<< reverse
	abcdef 123456
	>>
	@endcode
*/
NMEErr NMEPluginUppercase(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData);

/// NMEPlugin entry for table of plugins
#define NMEPluginUppercaseEntry \
	{"uppercase", kNMEPluginOptReparseOutput, NMEPluginUppercase, NULL}

#ifdef __cplusplus
}
#endif

#endif
