/**
 *	@file NMETest.h
 *	@brief Test style to check the correct nesting of NME style strings.
 *	@author Yves Piguet.
 *	@copyright 2011, Yves Piguet
 */

/* License: new BSD license (see NME.h) */

#ifndef __NMETest__
#define __NMETest__

#ifdef __cplusplus
extern "C" {
#endif

#include "NME.h"

enum
{
	kNMEErrSuperfluousEndTag = kNMEErr1stUser,
	kNMEErrBadNesting,
	kNMEErrMissingEndTag
};

/**	Test format to check the correct nesting of style strings.
	Each start or end of construct results in a separate line, starting
	with a + for start or a - for end. Corresponding lines should match
	and nest correctly.
*/
extern NMEOutputFormat const NMEOutputFormatTest;

/**	Test the output generated with NMEOutputFormatTest
	@param[in] output output generated with NMEOutputFormatTest
	@param[in] outputLength length of output in bytes
	@param[out] lineNumber last line processed
	@return kNMEErrOk for success, else error code
*/
NMEErr NMETest(NMEConstText output, NMEInt outputLength, NMEInt *lineNumber);

#ifdef __cplusplus
}
#endif

#endif
