/**
 *	@file NMETest.c
 *	@brief Test style to check the correct nesting of NME style strings.
 *	@author Yves Piguet.
 *	@copyright 2011, Yves Piguet
 */

/* License: new BSD license (see header file) */

#include "NMETest.h"

/** NMEEncodeURLFun function which encodes link to a URL for null output,
	producing no output.
	@param[in] link input characters (ignored)
	@param[in] linkLen length of link (ignored)
	@param[in,out] context current context (ignored)
	@param[in,out] data value specific to the callback (ignored)
	@return error code (always kNMEErrOk)
*/
static NMEErr encodeURLFunNull(NMEConstText link, NMEInt linkLen,
		NMEContext *context, void *data)
{
	return kNMEErrOk;
}

/** NMEEncodeCharFun function which encodes characters for null output, producing
	no output; the encodeCharData field is ignored (can be set to NULL).
	@param[in] src input characters
	@param[in] srcLen size of src in bytes
	@param[in,out] srcIx index in src (should be updated by one character)
	@param[in,out] context current context
	@param[in,out] data ignored 
	@return error code (kNMEErrOk for success)
	@see NMEOutputFormat
*/
static NMEErr encodeCharFunNull(NMEConstText src, NMEInt srcLen, NMEInt *srcIx,
		NMEContext *context,
		void *data)
{
	(*srcIx)++;
	return kNMEErrOk;
}

NMEOutputFormat const NMEOutputFormatTest =
{
	"",	// space
	0,	// indentSpaces
	10,	// defFontSize
	'%',	// ctrlChar
	"+;%{L}\n", "-;%{L}\n",	// doc
	10,	// highest heading level
	"+%%{l} %%H %{l} %{i};%{L}\n", "-%%{l} %%H %{l} %{i};%{L}\n",	// heading
	"+ par;%{L}\n", "- par;%{L}\n",	// par
	"",	// line break
	"+ pre;%{L}\n", "- pre;%{L}\n",	// pre
	"+ preline;%{L}\n", "- preline;%{L}\n",	// pre line
	FALSE,	// sublistInListItem
	"+%%{l} %%U %{l};%{L}\n", "-%%{l} %%U %{l};%{L}\n",	// UL
	"+%%{l} %%u %{l} %{i};%{L}\n", "-%%{l} %%u %{l} %{i};%{L}\n",	// UL line
	"+%%{l} %%O %{l};%{L}\n", "-%%{l} %%O %{l};%{L}\n",	// OL
	"+%%{l} %%o %{l} %{i};%{L}\n", "-%%{l} %%o %{l} %{i};%{L}\n",	// OL line
	"+%%{l} %%D %{l};%{L}\n", "-%%{l} %%D %{l};%{L}\n",	// DL
	"+%%{l} %%t %{l} %{i};%{L}\n", "-%%{l} %%t %{l} %{i};%{L}\n",	// DT
	"",	// emptyDT
	"+%%{l} %%d %{l} %{i};%{L}\n", "-%%{l} %%d %{l} %{i};%{L}\n",	// DD
	"+%%{l} %%I %{l};%{L}\n", "-%%{l} %%I %{l};%{L}\n",	// indented section
	"+%%{l} %%i %{l} %{i};%{L}\n", "-%%{l} %%i %{l} %{i};%{L}\n",	// indented par
	"+ T;%{L}\n", "- T;%{L}\n",	// table
	"+  t;%{L}\n", "-  t;%{L}\n",	// table row
	"+   th;%{L}\n", "-   th;%{L}\n",	// table header cell
	"+   td;%{L}\n", "-   td;%{L}\n",	// table normal cell
	"",	// hr
	"+*;%{L}\n", "-*;%{L}\n",	// bold
	"+/;%{L}\n", "-/;%{L}\n",	// italic
	"+_;%{L}\n", "-_;%{L}\n",	// underline
	"+^;%{L}\n", "-^;%{L}\n",	// superscript
	"+,;%{L}\n", "-,;%{L}\n",	// subscript
	"+#;%{L}\n", "-#;%{L}\n",	// monospace
	"+a;%{L}\n", "", "-a;%{L}\n", FALSE,	// link
	"+i;%{L}\n", "", "-i;%{L}\n", FALSE, TRUE,	// image
	NULL,	// interwikis
	encodeURLFunNull, NULL,	// encodeURLFun
	encodeCharFunNull, NULL,	// char encoder
	encodeCharFunNull, NULL,	// char pre encoder
	-1, NULL, NULL,	// no wordwrap
	NULL, NULL,	// char hook
	NULL, NULL, NULL, NULL,	// process hooks
	NULL,	// plugins
	NULL,	// autoconverts
	NULL, NULL	// getVar
};

NMEErr NMETest(NMEConstText output, NMEInt outputLength, NMEInt *lineNumber)
{
	// Output is parsed line by line. When an end-of-construct line is parsed,
	// it's compared against the last one still considered. If they match, the
	// start line is discarded; otherwise, an error is returned.
	
	NMEInt currentLine;	// offset of current line
	NMEInt i;	// used for comparison
	
#define kMaxDepth 32
	// 2 * kMaxNesting + style number + doc root + margin
	// (2* for lists + list items; margin to produce more useful output in case of bug)
	NMEInt startOffset[kMaxDepth];
	NMEInt depth;
	
	for (currentLine = 0, depth = 0; currentLine < outputLength; )
	{
		if (output[currentLine] == '-')
		{
			if (depth < 0)
				return kNMEErrSuperfluousEndTag;
			currentLine++;
			for (i = 0;
					currentLine + i < outputLength
						&& output[currentLine + i] != '\n' && output[currentLine + i] != ';';
					i++)
				if (output[currentLine + i] != output[startOffset[depth - 1] + i])
					return kNMEErrBadNesting;
			if (output[startOffset[depth - 1] + i] != output[currentLine + i])
				return kNMEErrBadNesting;
			currentLine += i;
			depth--;
		}
		else
		{
			if (depth >= kMaxDepth)
				return kNMEErrInternal;
			startOffset[depth++] = currentLine + 1;	// skip +
			// skip until semicolon
			while (currentLine < outputLength
					&& output[currentLine] != '\n' && output[currentLine] != ';')
				currentLine++;
		}
		
		// decode line number and skip end of line
		for (*lineNumber = 0;
				currentLine < outputLength && output[currentLine] != '\n';
				currentLine++)
			if (output[currentLine] >= '0' && output[currentLine] <= '9')
				*lineNumber = 10 * *lineNumber + output[currentLine] - '0';
		if (currentLine < outputLength && output[currentLine] == '\n')
			currentLine++;
	}
	
	if (depth > 0)
		return kNMEErrMissingEndTag;
	else
		return kNMEErrOk;
}
