/**
 *	@file NMEStyle.c
 *	@brief Hooks to collect style information separately.
 *	@author Yves Piguet.
 *	@copyright 2007-2011, Yves Piguet.
 */

/* License: new BSD license (see header file) */

#include "NMEStyle.h"

NMEOutputFormat const NMEOutputFormatBasicText =
{
	" ",	// space
	0,	// indentSpaces
	10,	// defFontSize
	'%',	// ctrlChar
	"", "",	// doc
	4,	// highest heading level
	"%%{i>0}%{i}. %%", "\n",	// heading
	"", "\n",	// par
	"\n",	// line break
	"", "",	// pre
	"", "\n",	// pre line
	FALSE,	// sublistInListItem
	"", "",	// UL
	"- ", "\n",	// UL line
	"", "",	// OL
	"%{i}. ", "\n",	// OL line
	"", "",	// DL
	"", " ",	// DT
	NULL,	// emptyDT
	"", "\n",	// DD
	"", "",	// indented section
	"", "\n",	// indented par
	"", "",	// table
	"", "\n",	// table row
	"", "\t",	// table header cell
	"", "\t",	// table normal cell
	"\n",	// hr
	"", "",	// bold
	"", "",	// italic
	"", "",	// underline
	"", "",	// superscript
	"", "",	// subscript
	"", "",	// monospace
	"", "", NULL, FALSE,	// link
	"", "", NULL, FALSE, FALSE,	// image
	NULL,	// interwikis
	NULL, NULL,	// encodeURLFun
	NULL, NULL,	// char encoder
	NULL, NULL,	// char pre encoder
	-1, NULL, NULL,	// no wordwrap
	NULL, NULL, NULL, NULL,	// process hooks
	NULL,	// plugins
	NULL	// autoconverts
};

void NMEStyleInit(NMEStyleTable *table, NMEInt size,
		NMEBoolean convertOffsetsToUnicode)
{
	table->tableSize = (size - (sizeof(NMEStyleTable) - sizeof(NMEStyleSpan)))
			/ sizeof(NMEStyleSpan);
	table->n = 0;
	table->convertOffsetsToUnicode = convertOffsetsToUnicode;
	table->depth = 0;
}

/** Convert style markup string to a NMEStyleEnum value.
	@param[in] markup markup string, as provided by NMEProcessHookFun
	@param[in] level heading or list level (1 = topmost)
	@return style
*/
static NMEStyleEnum decodeStyle(NMEConstText markup, NMEInt level)
{
	if (level == kNMEHookLevelSpan)	// span
		switch (markup[0])
		{
			case '*':
				return kNMEStyleCharBold;
			case '/':
				return kNMEStyleCharItalic;
			case '_':
				return kNMEStyleCharUnderline;
			case '^':
				return kNMEStyleCharSuperscript;
			case ',':
				return kNMEStyleCharSubscript;
			case '#':
				return kNMEStyleCharMonospace;
			case '[':
				return kNMEStyleCharLink;
			case '{':
				return kNMEStyleCharImage;
			default:
				return kNMEStyleCharPlain;
		}
	else	// par
		switch (markup[0])
		{
			case '{':
				return kNMEStyleCharMonospace;
			case '=':
				return kNMEStyleParHeading;
			case '*':
				return kNMEStyleParUL;
			case '#':
				return kNMEStyleParOL;
			case ';':
				return markup[1] == ':' ? kNMEStyleParDT : kNMEStyleParDL;
			case ':':
				return kNMEStyleParIndentedPar;
			case '|':
				return kNMEStyleParTable;
			default:
				return kNMEStyleParPlain;
		}
}

NMEErr NMEStyleSpanHook(NMEInt level,
		NMEInt item,
		NMEBoolean enter,
		NMEConstText markup,
		NMEInt srcIndex,
		NMEInt srcLineNumber,
		NMEContext *context,
		void *data)
{
	NMEStyleTable *table = (NMEStyleTable *)data;
	NMEStyleEnum style = decodeStyle(markup, level);
	NMEStyleEnum substyle = kNMEStyleCharPlain;
	
	if (style == kNMEStyleCharPlain)
		return kNMEErrOk;
	else if (style == kNMEStyleParDL && markup[1] == '\0')
		substyle = kNMEStyleCharDT;
	else if (style == kNMEStyleParTable && markup[1] == '=')
		substyle = kNMEStyleCharTH;
	
	if (enter)
	{
		if (table->n >= table->tableSize)
			return kNMEErrStyleTableTooSmall;
		table->span[table->n].begin
				= table->convertOffsetsToUnicode
						? NMECurrentOutputIndexUCS16(context)
						: NMECurrentOutputIndex(context);
		table->span[table->n].style = style;
		table->span[table->n].level = level;
		NMECurrentLink(context,
				&table->span[table->n].linkOffset, &table->span[table->n].linkLength);
		table->spanStack[table->depth++] = table->n;
		table->n++;
		if (substyle != kNMEStyleCharPlain)
		{
			if (table->n >= table->tableSize)
				return kNMEErrStyleTableTooSmall;
			table->span[table->n] = table->span[table->n - 1];
			table->span[table->n].style = substyle;
			table->spanStack[table->depth++] = table->n;
			table->n++;
		}
	}
	else
	{
		NMEInt end = table->convertOffsetsToUnicode
				? NMECurrentOutputIndexUCS16(context)
				: NMECurrentOutputIndex(context);
		
		if (table->depth + (substyle != kNMEStyleCharPlain ? 1 : 0) <= 0)
			return kNMEErrInternal;
		if (substyle != kNMEStyleCharPlain)
			table->span[table->spanStack[--table->depth]].end = end;
		table->span[table->spanStack[--table->depth]].end = end;
	}
	
	return kNMEErrOk;
}
