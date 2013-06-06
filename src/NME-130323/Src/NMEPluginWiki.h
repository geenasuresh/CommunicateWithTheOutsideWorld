/**
 *	@file NMEPluginWiki.h
 *	@brief NME optional plugin for wiki extensions.
 *	@author Yves Piguet.
 *	@copyright 2009-2011, Yves Piguet.
 */

/* License: new BSD license (see NME.h) */

/**	@page nmepluginwiki NME Wiki Plugin
 *
 *	Nyctergatis Markup Engine (NME) Wiki Plugin
 *
 *	@author Yves Piguet
 *	@see http://www.nyctergatis.com (home of NME project)
 *	and http://www.wikicreole.org (Creole site).
 *
 *	The Wiki plugins are optional plugins and hooks to provide additional
 *	capabilities useful in wikis:
 *	- &lt;&lt; keywords aaa, bbb, ... >> adds keywords which can be extracted
 *	- &lt;&lt; execute method arg1 arg2 ... >> adds a method call which can
 *	be extracted; its execution is up to the application
 *	- &lt;&lt; date YYYY-MM-DD hh:mm >> adds a date which can be extracted
 *	- top-level headings can be extracted
 *
 *	Each piece of extracted information is written to the output stream in
 *	a separate line which begins with an identifier (respectively Keyword,
 *	Execute, Date, or Time), a semicolon and a space. Typically, the
 *	document should be converted with NMEProcess using the NMEOutputFormatNull
 *	format.
 */

#ifndef __NMEPluginWiki__
#define __NMEPluginWiki__

#ifdef __cplusplus
extern "C" {
#endif

#include "NME.h"

/** Plugin implementation for keywords (plugin's data: comma-separated keywords);
	write each keyword on a separate line (should be used with NMEOutputFormatNull)
	@param[in] name plugin name, such as "keywords"
	@param[in] nameLen length of name
	@param[in] data data text
	@param[in] dataLen length of data
	@param[in,out] context current context
	@param[in] userData pointer passed from the parser, as specified in NMEPlugin
	@return error code (kNMEErrOk for success)
	@test @code
	<< keywords wiki, creole, markup >>
	@endcode
*/
NMEErr NMEPluginKeywords(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData);

/// NMEPlugin entry for table of plugins
#define NMEPluginKeywordsEntry \
	{"keywords", kNMEPluginOptBetweenPar, \
		NMEPluginKeywords, NULL}

/** Plugin implementation for execute (plugin's data: method name and arguments);
	write data without leading and trailing spaces on a separate line (should be used with NMEOutputFormatNull)
	@param[in] name plugin name, such as "execute"
	@param[in] nameLen length of name
	@param[in] data data text
	@param[in] dataLen length of data
	@param[in,out] context current context
	@param[in] userData pointer passed from the parser, as specified in NMEPlugin
	@return error code (kNMEErrOk for success)
	@test @code
	<< execute makeCalendar 2009 12 >>
	@endcode
*/
NMEErr NMEPluginExecute(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData);

/// NMEPlugin entry for table of plugins
#define NMEPluginExecuteEntry \
	{"execute", kNMEPluginOptBetweenPar, \
		NMEPluginExecute, NULL}

/** Plugin implementation for date (plugin's data: date and time);
	write each date on a separate line (should be used with NMEOutputFormatNull)
	@param[in] name plugin name, such as "keywords"
	@param[in] nameLen length of name
	@param[in] data data text
	@param[in] dataLen length of data
	@param[in,out] context current context
	@param[in] userData comma-separated lowercase month names (multiple of 12 values
	for multiple languages, etc., such as "january,february,...,december,janvier,f\xc3\xa9vrier,...")
	@return error code (kNMEErrOk for success)
	@test @code
	<< date 2 oct 2003 >>
	<< date 2 oct 2003 12:45 >>
	<< date 2 oct 2003 3 pm >>
	@endcode
*/
NMEErr NMEPluginDate(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData);

#define NMEMonthNameEn \
	"january,february,march,april,may,june,july,august,september,october,november,december,"
#define NMEMonthNameFr \
	"janvier,f\xc3\xa9vrier,mars,avril,mai,juin,juillet,ao\xc3\xbbt,septembre,octobre,novembre,d\xc3\xa9""cembre,"
#define NMEMonthNameSp \
	"enero,febrero,marzo,abril,mayo,junio,julio,agosto,septiembre,octubre,noviembre,diciembre,"
#define NMEMonthNameIt \
	"gennaio,febbraio,marzo,aprile,maggio,giugno,luglio,agosto,settembre,ottobre,novembre,dicembre,"
#define NMEMonthNameDe \
	"januar,februar,m\xc3\xa4rz,april,mai,juni,juli,august,september,oktober,november,dezember,"

/// NMEPlugin entry for table of plugins
#define NMEPluginDateEntry(months) \
	{"date", kNMEPluginOptBetweenPar, \
		NMEPluginDate, months}

#ifdef __cplusplus
}
#endif

#endif
