/**
 *	@file NMEPluginWiki.c
 *	@brief NME optional plugin for wiki extensions (collect metadata).
 *	@author Yves Piguet.
 *	@copyright 2009-2011, Yves Piguet.
 */

/* License: new BSD license (see NME.h) */

#include "NMEPluginWiki.h"
#include <string.h>
#include <stdio.h>

#define isWhiteSpace(c) ((c) <= ' ' && (c) >= '\0')
#define isDigit(c) ((c) >= '0' && (c) <= '9')
#define isAlpha(c) ((c) >= 'a' && (c) <= 'z' || (c) >= 'A' && (c) <= 'Z')
#define digit(c) ((c) - '0')
#define lower(c) ((c) >= 'A' && (c) <= 'Z' ? (c) + 32 : (c))

NMEErr NMEPluginKeywords(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData)
{
	int i, j, k;
	(void)name;
	(void)nameLen;
	(void)userData;
	
	// read keywords in data
	for (i = 0; i < dataLen; i = j + 1)
	{
		// skip white spaces
		while (i < dataLen && isWhiteSpace(data[i]))
			i++;
		// find next comma
		for (j = i; j < dataLen && data[j] != ','; j++)
			;
		// discard trailing white spaces
		for (k = j; k > i && isWhiteSpace(data[k - 1]); k--)
			;
		// output keyword
		NMEAddRawString("Keyword: ", -1, context);
		NMEAddRawString(data + i, k - i, context);
		NMEAddRawString("\n", -1, context);
	}
	
	return kNMEErrOk;
}

NMEErr NMEPluginExecute(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData)
{
	int i, k;
	(void)name;
	(void)nameLen;
	(void)userData;
	
	// skip white spaces
	for (i = 0; i < dataLen && isWhiteSpace(data[i]); i++)
		;
	// discard trailing white spaces
	for (k = dataLen; k > i && isWhiteSpace(data[k - 1]); k--)
		;
	// output
	NMEAddRawString("Execute: ", -1, context);
	NMEAddRawString(data + i, k - i, context);
	NMEAddRawString("\n", -1, context);
	
	return kNMEErrOk;
}

/**	Check if the text at a given offset matches a template; what follows immediately
	(if not end of text) must not be a digit or a letter.
	@param[in] data text
	@param[in] dataLen length of data in bytes
	@param[in] i offset in data
	@param[in] template template null-terminated string; '0' matches digits,
	'a' matches characters, and anything else matches itself
	@return TRUE if match, else FALSE
*/
static NMEBoolean matchTemplate(NMEConstText data, NMEInt dataLen, NMEInt i,
		NMEConstText template)
{
	NMEInt j;
	
	for (j = 0; template[j]; j++)
		if (i + j >= dataLen
				|| (template[j] == '0' ? !isDigit(data[i + j])
					: template[j] == 'a' ? !isAlpha(data[i + j])
					: template[j] != data[i + j]))
			return FALSE;
	return i + j >= dataLen || !isDigit(data[i + j]) && !isAlpha(data[i + j]);
}

/**	Write an unsigned integer of up to 4 digits.
	@param[in] padding if positive, number of digits including leading zeros
*/
static void writeInt(NMEInt n, NMEInt padding, NMEContext *context)
{
	NMEChar str[5];
	NMEInt i;
	
	if (padding <= 0)	// compute number of digits in n (at least 1)
		for (i = 10, padding = 1; n >= i; i *= 10)
			padding++;
	
	for (i = 0; i < padding; i++, n /= 10)
		str[padding - i - 1] = '0' + n % 10;
	str[padding] = '\0';
	
	NMEAddRawString(str, -1, context);
}

NMEErr NMEPluginDate(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData)
{
	int year = 0, month = 0, day = 0, hour = -1, min = 0;
	NMEInt i;
	NMEConstText monthNames = (NMEConstText)userData;
	(void)name;
	(void)nameLen;
	
	// read date and time in data
	for (i = 0; i < dataLen; )
	{
		// skip spaces
		while (i < dataLen && isWhiteSpace(data[i]))
			i++;
		// check format
		if (matchTemplate(data, dataLen, i, "00:00"))
		{
			// hh:mm
			hour = 10 * digit(data[i]) + digit(data[i + 1]);
			min = 10 * digit(data[i + 3]) + digit(data[i + 4]);
			i += 5;
		}
		else if (matchTemplate(data, dataLen, i, "0:00"))
		{
			// h:mm
			hour = digit(data[i]);
			min = 10 * digit(data[i + 2]) + digit(data[i + 3]);
			i += 4;
		}
		else if (matchTemplate(data, dataLen, i, "0000-00-00"))
		{
			// yyyy-mm-dd
			year = 1000 * digit(data[i]) + 100 * digit(data[i + 1])
					+ 10 * digit(data[i + 2]) + digit(data[i + 3]);
			month = 10 * digit(data[i + 5]) + digit(data[i + 6]);
			day = 10 * digit(data[i + 8]) + digit(data[i + 9]);
			i += 10;
		}
		else if (matchTemplate(data, dataLen, i, "00.00.0000"))
		{
			// dd.mm.yyyy
			day = 10 * digit(data[i]) + digit(data[i + 1]);
			month = 10 * digit(data[i + 3]) + digit(data[i + 4]);
			year = 1000 * digit(data[i + 6]) + 100 * digit(data[i + 7])
					+ 10 * digit(data[i + 8]) + digit(data[i + 9]);
			i += 10;
		}
		else if (matchTemplate(data, dataLen, i, "00/00/0000"))
		{
			// mm/dd/yyyy
			month = 10 * digit(data[i]) + digit(data[i + 1]);
			day = 10 * digit(data[i + 3]) + digit(data[i + 4]);
			year = 1000 * digit(data[i + 6]) + 100 * digit(data[i + 7])
					+ 10 * digit(data[i + 8]) + digit(data[i + 9]);
			i += 10;
		}
		else if (matchTemplate(data, dataLen, i, "0000"))
		{
			// yyyy
			year = 1000 * digit(data[i]) + 100 * digit(data[i + 1])
					+ 10 * digit(data[i + 2]) + digit(data[i + 3]);
			i += 4;
		}
		else if (matchTemplate(data, dataLen, i, "00"))
		{
			// dd
			day = 10 * digit(data[i]) + digit(data[i + 1]);
			i += 2;
		}
		else if (matchTemplate(data, dataLen, i, "0"))
		{
			// d
			day = digit(data[i]);
			i += 1;
		}
		else if (matchTemplate(data, dataLen, i, "pm"))
		{
			if (hour < 12 && hour >= 0)
				hour += 12;
			i += 2;
		}
		else
		{
			if (monthNames && i + 2 < dataLen)
			{
				// try to find a month name (at least 3 char)
				NMEInt m;	// month index in monthNames
				NMEInt j;	// char index in monthNames
				NMEInt k;	// i+k = index in data
				
				for (m = j = 0; monthNames[j]; m++)
				{
					// match?
					for (k = 0;
							i + k < dataLen && lower(data[i + k]) == monthNames[j + k];
							k++)
						;
					if (k >= 3)
					{
						i += k;
						month = 1 + m % 12;
						goto next;
					}
					
					// skip month name in monthNames
					while (monthNames[j] && monthNames[j] != ',')
						j++;
					if (monthNames[j] == ',')
						j++;
				}
			}
			// no month name found
			i++;
next:
			;
		}
	}
	
	// write date
	if (year > 0 && month > 0 && day > 0)
	{
		NMEAddRawString("Date: ", -1, context);
		writeInt(year, 4, context);
		NMEAddRawString("-", -1, context);
		writeInt(month, 2, context);
		NMEAddRawString("-", -1, context);
		writeInt(day, 2, context);
		NMEAddRawString("\n", -1, context);
	}
	
	// write time
	if (hour >= 0)
	{
		NMEAddRawString("Time: ", -1, context);
		writeInt(hour, 2, context);
		NMEAddRawString(":", -1, context);
		writeInt(min, 2, context);
		NMEAddRawString("\n", -1, context);
	}
	
	return kNMEErrOk;
}
