/**
 *	@file NMEStyle.h
 *	@brief Hooks to collect style information separately.
 *	@author Yves Piguet.
 *	@copyright 2007-2011, Yves Piguet
 *
 *	@section Introduction Introduction
 *
 *	To display text with NME markup, there are two possibilities:
 *	- Convert NME input to some other markup which can be displayed
 *    directly, such as HTML, XML or RTF. This requires only a call
 *    to NMEProcess, maybe with custom NMEOutputFormat. For example,
 *    HTML could be used with a WebBrowser ActiveX control in Windows,
 *    or RTF with replaceCharactersInRange:withRTF: in Cocoa (Mac OS X).
 * 	- Convert NME to text, collecting information about style separately,
 *    and apply this style information to spans of text.
 *
 *  NMEStyle offers support for this latter one. The NMEOutputFormat should
 *  produce pure text with end-of-line conventions appropriate for the
 *  platform, typically without wordwrap.
 *
 *	@section Usage Usage
 *
 *	The code sample below is reduced to the minimum. See NME.h to see
 *	how memory allocation can be handled.
 *
 *	@code
 *	#include "NME.h"
 *	#include "NMEStyle.h"
 *
 *	NMEText input;
 *	NMEInt inputLength;
 *	(read source of length inputLength into input)
 *	NMEInt size = ...; // size of buffers, typically initialized to 1024+2*inputLength
 *	NMEText buf;
 *	buf = malloc(size);
 *	NMEText output;
 *	NMEInt outputLength, outputLengthUCS16;
 *	NMEErr err;
 *	NMEOutputFormat f;
 *	f = NMEOutputFormatBasicText;
 *	f.spanHookFun = NMEStyleSpanHook;
 *	f.parHookFun = NMEStyleSpanHook;
 *	f.hookData = malloc(size);
 *	NMEStyleInit((NMEStyleTable *)f.hookData, bufSize, TRUE);
 *	for (;;)
 *	{
 *		err = NMEProcess(input, inputLength,
 *			buf, size,
 *			kNMEProcessOptDefault, "\n", &f, 0,
 *			&output, &outputLength, &outputLengthUCS16);
 *		if (err == kNMEErrNotEnoughMemory	// unlikely
 *				|| || err == kNMEErrStyleTableTooSmall)
 *		{
 *			(increase size)
 *			(realloc buf and f.hookData)
 *		}
 *		else
 *			break;
 *	}
 *	if (err == kNMEErrOk)
 *	{
 *		(apply styles in (NMEStyleTable *)f.hookData)
 *		(write outputLength first bytes of output[])
 *	}
 *	else
 *		(handle error)
 *	free(buf);
 *	free(f.hookData);
 *	@endcode
 */

/* License: new BSD license (see NME.h) */

#ifndef __NMEStyle__
#define __NMEStyle__

#ifdef __cplusplus
extern "C" {
#endif

#include "NME.h"

/// Error codes
enum
{
	kNMEErrStyleTableTooSmall = kNMEErr1stNMEOpt
};

/// Style
typedef enum NMEStyleEnum
{
	kNMEStyleCharPlain = 0,	///< plain (not used in table)
	kNMEStyleCharBold,	///< bold
	kNMEStyleCharItalic,	///< italic
	kNMEStyleCharUnderline,	///< underline
	kNMEStyleCharSuperscript,	///< superscript
	kNMEStyleCharSubscript,	///< subscript
	kNMEStyleCharMonospace,	///< monospace
	kNMEStyleCharLink,	///< hypertext link
	kNMEStyleCharImage,	///< image
	kNMEStyleCharDT,	///< definition list title
	kNMEStyleCharTH,	///< table heading
	
	kNMEStyleParPlain,	///< plain paragraph
	kNMEStyleParHeading,	///< heading of any level
	kNMEStyleParTable,	///< table row
	kNMEStyleParUL,
	kNMEStyleParOL,
	kNMEStyleParDL,
	kNMEStyleParDT,
	kNMEStyleParIndentedPar,
	
	kNMEStyleCount	///< number of styles
} NMEStyleEnum;

/// Style span
typedef struct
{
	NMEInt begin;	///< offset of the span beginning
	NMEInt end;	///< offset of the span end
	NMEStyleEnum style;	///< span style
	NMEInt level;	///< level of headings or lists
	NMEInt linkOffset;	///< link offset in source (kNMEStyleCharLink/kNMEStyleCharImage)
	NMEInt linkLength;	///< link length in source (kNMEStyleCharLink/kNMEStyleCharImage)
} NMEStyleSpan;

/// Table of style spans
typedef struct
{
	NMEInt tableSize;	///< number of entries in table
	NMEInt n;	///< number of filled entries in table
	NMEBoolean convertOffsetsToUnicode;	/**< TRUE to have span locations in
			UCS-16 chararcters */
	NMEInt depth;	///< number of elements in spanStack[]
	NMEInt spanStack[kNMEStyleCount];	///< stack of pending styles
	NMEStyleSpan span[1];	///< spans (actual size is tableSize)
} NMEStyleTable;

/// Format strings for basic text output, suitable for separate style
extern NMEOutputFormat const NMEOutputFormatBasicText;

/** Initialize NMEStyleTable.
	@param[out] table table of style spans
	@param[in] size size of table in bytes
	@param[in] convertOffsetsToUnicode TRUE if original text is in
	UTF-8 and span locations in table must be converted to values for
	UCS-16 text (16-bit unicode), FALSE if span locations in table
	are in bytes (independant from the chararcter encoding)
*/
void NMEStyleInit(NMEStyleTable *table, NMEInt size,
		NMEBoolean convertOffsetsToUnicode);

/** Process hook for storing styles in a table.
	@param[in] level heading or list level (1 = topmost)
	@param[in] item list item or heading counter
	@param[in] enter TRUE when entering construct, FALSE when exiting
	@param[in] markup null-terminated string for initial markup ("p" for par,
	"*" for unnumbered list of any level, "//" for italic, "{{{" for preformatted)
	@param[in] srcIndex current index in source code
	@param[in] srcLineNumber current line number in source code
	@param[in,out] context current context
	@param[in,out] data table of styles
	@return error code (kNMEErrOk for success)
*/
NMEErr NMEStyleSpanHook(NMEInt level,
		NMEInt item,
		NMEBoolean enter,
		NMEConstText markup,
		NMEInt srcIndex,
		NMEInt srcLineNumber,
		NMEContext *context,
		void *data);

#ifdef __cplusplus
}
#endif

#endif
