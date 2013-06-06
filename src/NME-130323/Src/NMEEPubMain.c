/**
 *	@file NMEEPubMain.c
 *	@brief EPub creation based on NME.
 *	@author Yves Piguet.
 *	@copyright 2010-2011, Yves Piguet.
 */

/* License: new BSD license (see NME.h) */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zip.h"
#include "NME.h"
#include "NMEAutolink.h"
#include "NMEEPub.h"
#include "NE.h"

/// Fixed size allocated for source :-(
#define SIZE (8 * 1024 * 1024L)

#define kNMENEEndnoteError kNMEErr1stUser

static NMEBoolean debug = FALSE;

/// Table of autoconvert functions
static NMEAutoconvert autoconverts[16];

/// Table of interwiki definitions
static NMEInterwiki const interwikis[] =
{
	{"Dictionary:",
		"http://www.dict.org/bin/Dict?Database=*&Form=Dict1&Strategy=*&Query="},
	{"Foldoc:", "http://www.foldoc.org/foldoc/foldoc.cgi?"},
	{"Google:", "http://www.google.com/search?q="},
	{"WhoIs:", "http://www.whois.sc/"},
	{"WikiPedia:", "http://en.wikipedia.org/wiki/"},
	{NULL, NULL}
};

static void DisplayUsageAndExit(char const *progName, int status)
{
	fprintf(stderr, "Usage: %s [options] -o epubFilename file1 ...\n"
			"Required arguments:\n"
			"-o epubFilename   specified the EPUB file to create (e.g. book.epub)\n"
			"file1 ...         NME files constituting  the contents of the book\n"
			"Options:\n"
			"--1eol            single eol as paragraph breaks\n"
			"--2eol            double eol as paragraph breaks (default)\n"
			"--autocclink      automatic conversion of camelCase words to links\n"
			"--autourllink     automatic conversion of URLs to links\n"
			"--debug           XML debug format, sublists outside list items\n"
			"--headernum1      numbering of level-1 headers\n"
			"--headernum2      numbering of level-2 headers\n",
			progName);
	exit(status);
}

///	User data for par hook, to process titles and images
typedef struct
{
	NMEOutputFormat *outputFormat;
	NE *ne;
	
	// titles
	char const *filename;
	int titleSrcOffset;
	
	// images
	NEBoolean inImageMarkup;	///< TRUE if in image markup
} HookData;

/**	Paragraph hook to extract titles.
	@param[in] level heading or list level (1 = topmost, par=kNMEHookLevelPar,
	span=kNMEHookLevelSpan)
	@param[in] item list item or heading counter
	@param[in] enter TRUE when entering construct, FALSE when exiting
	@param[in] markup null-terminated string for initial markup ("p" for par,
	"*" for unnumbered list of any level, "//" for italic, "{{{" for preformatted)
	@param[in] srcIndex current index in source code
	@param[in] srcLineNumber current line number in source code
	@param[in,out] context current context
	@param[in,out] data value specific to the callback
	@return error code (kNMEErrOk for success)
*/
static NMEErr parHookTOC(NMEInt level,
		NMEInt item,
		NMEBoolean enter,
		NMEConstText markup,
		NMEInt srcIndex,
		NMEInt srcLineNumber,
		NMEContext *context,
		void *data)
{
	HookData *d = (HookData *)data;
	
	if (markup[0] == '=' && level == 1)
		if (enter)
		{
			NMEResetOutput(context);
			*d->outputFormat = NMEOutputFormatNull;
			d->outputFormat->space = " ";
			d->outputFormat->encodeCharFun = NULL;
			d->outputFormat->parHookFun = parHookTOC;
			d->outputFormat->hookData = data;
			d->titleSrcOffset = NMECurrentInputIndex(context);
		}
		else
		{
			NMEConstText output;
			NMEInt outputLength;
			char title[512], url[512];
			
			NMECurrentOutput(context, &output, &outputLength);
			if (outputLength > 511)
				outputLength = 511;
			sprintf(title, "%.*s", outputLength, output);
			sprintf(url, "%s#h%d", d->filename, d->titleSrcOffset);
			NEAddTOCEntry(d->ne, title, url, 1);
			
			*d->outputFormat = NMEOutputFormatNull;
			d->outputFormat->parHookFun = parHookTOC;
			d->outputFormat->hookData = data;
		}
	
	return kNMEErrOk;
}

/**	Span hook to extract image URLs (set a flag when inside image markup,
	so that the encodeURL callback knows when it's given an image URL).
	@param[in] level heading or list level (1 = topmost, par=kNMEHookLevelPar,
	span=kNMEHookLevelSpan)
	@param[in] item list item or heading counter
	@param[in] enter TRUE when entering construct, FALSE when exiting
	@param[in] markup null-terminated string for initial markup ("p" for par,
	"*" for unnumbered list of any level, "//" for italic, "{{{" for preformatted)
	@param[in] srcIndex current index in source code
	@param[in] srcLineNumber current line number in source code
	@param[in,out] context current context
	@param[in,out] data value specific to the callback
	@return error code (kNMEErrOk for success)
*/
static NMEErr spanHookImg(NMEInt level,
		NMEInt item,
		NMEBoolean enter,
		NMEConstText markup,
		NMEInt srcIndex,
		NMEInt srcLineNumber,
		NMEContext *context,
		void *data)
{
	HookData *d = (HookData *)data;
	
	if (!strcmp(markup, "{{"))
		d->inImageMarkup = enter;
	
	return kNMEErrOk;
}

/**	Encode URL callback used to get an image list.
	@param[in] link input characters
	@param[in] linkLen length of link
	@param[in] dest address of encoded text
	@param[in] destSize size of dest
	@param[in,out] destIx index in dest (should be updated by encoded character length)
	@param[in,out] data value specific to the callback
	@return error code (kNMEErrOk for success)
*/
static NMEErr encodeURL(NMEConstText link, NMEInt linkLen,
		NMEContext *context, void *data)
{
	HookData *d = (HookData *)data;
	
	if (d->inImageMarkup)
		NEAddOther(d->ne, link, linkLen, NULL);
	
	return NMEAddRawString(link, linkLen, context);	// no conversion
}

/**	Plugin for NE endnotes. Contents is NME code.
	@param[in] name plugin name, such as "endnote"
	@param[in] nameLen length of name
	@param[in] data data text (NME code)
	@param[in] dataLen length of data
	@param[in,out] context current context
	@param[in] userData NE (userdata specified when the plugin is installed)
	@return error code (kNMEErrOk for success)
	@test @code
	Some text<< endnote This is an endnote with **bold** and //italic//. >>.
	@endcode
*/
NMEErr PluginEndnote(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData)
{
	NMEText buf, output;
	NMEInt bufSize, outputLen;
	NMEErr nmeerr;
	NEErr neerr;
	char const *refLink;
	
	NMEGetTempMemory(context, &buf, &bufSize);
	nmeerr = NMEProcess(data, dataLen,
			buf, bufSize,
			kNMEProcessOptNoPreAndPost, "\n", &NMEOutputFormatOPSXHTML, 0,
			&output, &outputLen, NULL);
	if (nmeerr != kNMEErrOk)
		return nmeerr;
	
	neerr = NEAddEndnote((NEPtr)userData,
		output, outputLen,
		((NEPtr)userData)->currentDoc, -1,
		&refLink);
	if (neerr != kNEErrOk)
		return kNMENEEndnoteError;
	
	// insert link
	if (!NMEAddString(refLink, -1, '\0', context))
		return kNMEErrNotEnoughMemory;
	
	return kNMEErrOk;
}

/**	Plugin for NE metadata.
	@param[in] name plugin name ("title", "author", "identifier", "language",
	"subject", "description", "publisher", "date", "source", or "rights")
	@param[in] nameLen length of name
	@param[in] data metadata value
	@param[in] dataLen length of data
	@param[in,out] context current context
	@param[in] userData NE (userdata specified when the plugin is installed)
	@return error code (kNMEErrOk for success)
	@test @code
	<< title My Book >>
	<< author Author Name >>
	<< date 2010-08 >>
	<< identifier com.example.book12345 >>
	@endcode
*/
NMEErr PluginMeta(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData)
{
	NEPtr ne = (NEPtr)userData;
	
	// remove trailing spaces
	while (dataLen > 0 && data[dataLen - 1] <= ' ' && data[dataLen - 1] >= '\0')
		dataLen--;
	
	if (!strncmp(name, "title", nameLen))
		NEAddMetadata(ne, kNEMetaTitle, data, dataLen);
	else if (!strncmp(name, "author", nameLen))
		NEAddMetadata(ne, kNEMetaCreator, data, dataLen);
	else if (!strncmp(name, "identifier", nameLen))
		NEAddMetadata(ne, kNEMetaIdentifier, data, dataLen);
	else if (!strncmp(name, "language", nameLen))
		NEAddMetadata(ne, kNEMetaLanguage, data, dataLen);
	else if (!strncmp(name, "subject", nameLen))
		NEAddMetadata(ne, kNEMetaSubject, data, dataLen);
	else if (!strncmp(name, "description", nameLen))
		NEAddMetadata(ne, kNEMetaDescription, data, dataLen);
	else if (!strncmp(name, "publisher", nameLen))
		NEAddMetadata(ne, kNEMetaPublisher, data, dataLen);
	else if (!strncmp(name, "date", nameLen))
		NEAddMetadata(ne, kNEMetaDate, data, dataLen);
	else if (!strncmp(name, "source", nameLen))
		NEAddMetadata(ne, kNEMetaSource, data, dataLen);
	else if (!strncmp(name, "rights", nameLen))
		NEAddMetadata(ne, kNEMetaRights, data, dataLen);
	else if (!strncmp(name, "cover", nameLen))
		NESetCoverImage(ne, data, dataLen);
	return kNMEErrOk;
}

/**	Plugin for NE guide to structural components.
	@param[in] name plugin name ("guide")
	@param[in] nameLen length of name (5)
	@param[in] data component name as defined in OPF 2.0 ("cover", "title-page",
	"toc", "index", "glossary", "acknowldgements", "bibliography", "colophon",
	"copyright-page", "dedication", "epigraph", "foreword", "loi", "lot", "notes",
	"preface", "text")
	@param[in] dataLen length of data
	@param[in,out] context current context
	@param[in] userData NE (userdata specified when the plugin is installed)
	@return error code (kNMEErrOk for success)
	@test @code
	<< guide cover >>
	{cover.jpg}
	<< guide preface >>
	= Preface
	...
	<< guide text >>
	= Chapter 1
	...
	@endcode
*/
NMEErr PluginGuide(NMEConstText name, NMEInt nameLen,
		NMEConstText data, NMEInt dataLen,
		NMEContext *context,
		void *userData)
{
	
	return kNMEErrOk;
}

/// Application entry point
int main(int argc, char **argv)
{
	char const *epubFilename = NULL;
	char epubFilenameStr[512];
	FILE *fp;
	NMEText src = NULL, buf, dest;
	NMEInt srcLen, destLen;
	NMEOutputFormat outputFormat;
	NMEInt options = kNMEProcessOptDefault | kNMEProcessOptXRef;
	NMEBoolean autoURLLink = FALSE, autoCCLink = FALSE;
	int i;
	int iFiles;
	char const *imgFilename;
	int imgFilenameLength;
	NEErr neerr;
	NMEErr nmeerr;
	NE ne;
	HookData hookData;
	NMEPlugin plugins[] =
	{
		{"endnote", kNMEPluginOptDefault, PluginEndnote, NULL},
		{"title", kNMEPluginOptDefault, PluginMeta, NULL},
		{"author", kNMEPluginOptDefault, PluginMeta, NULL},
		{"identifier", kNMEPluginOptDefault, PluginMeta, NULL},
		{"language", kNMEPluginOptDefault, PluginMeta, NULL},
		{"subject", kNMEPluginOptDefault, PluginMeta, NULL},
		{"description", kNMEPluginOptDefault, PluginMeta, NULL},
		{"publisher", kNMEPluginOptDefault, PluginMeta, NULL},
		{"date", kNMEPluginOptDefault, PluginMeta, NULL},
		{"source", kNMEPluginOptDefault, PluginMeta, NULL},
		{"rights", kNMEPluginOptDefault, PluginMeta, NULL},
		{"cover", kNMEPluginOptDefault, PluginMeta, NULL},
		{"guide", kNMEPluginOptDefault, PluginGuide, NULL},
		NMEPluginTableEnd
	};
	
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-o") && i + 1 < argc)
			epubFilename = argv[++i];
		else if (!strcmp(argv[i], "--1eol"))
			options |= kNMEProcessOptNoMultilinePar;
		else if (!strcmp(argv[i], "--2eol"))
			options &= ~kNMEProcessOptNoMultilinePar;
		else if (!strcmp(argv[i], "--autocclink"))
			autoCCLink = TRUE;
		else if (!strcmp(argv[i], "--autourllink"))
			autoURLLink = TRUE;
		else if (!strcmp(argv[i], "--headernum1"))
			options |= kNMEProcessOptH1Num;
		else if (!strcmp(argv[i], "--headernum2"))
			options |= kNMEProcessOptH2Num;
		else if (!strcmp(argv[i], "--debug"))
			debug = TRUE;
		else if (!strcmp(argv[i], "--strictcreole"))
			options |= kNMEProcessOptNoUnderline | kNMEProcessOptNoMonospace
					| kNMEProcessOptNoSubSuperscript | kNMEProcessOptNoIndentedPar
					| kNMEProcessOptNoDL | kNMEProcessOptVerbatimMono;
		else if (!strcmp(argv[i], "--help"))
			DisplayUsageAndExit(argv[0], 0);
		else if (argv[i][0] == '-')
		{
			fprintf(stderr, "Unknown option %s\n", argv[i]);
			DisplayUsageAndExit(argv[0], 1);
		}
		else
			break;
	
	if (i == argc)
	{
		fprintf(stderr, "No document file.\n");
		DisplayUsageAndExit(argv[0], 1);
	}
	
	iFiles = i;
	
	if (!epubFilename && !debug)
	{
		strncpy(epubFilenameStr, argv[iFiles], 505);
		epubFilenameStr[505] = '\0';
		for (i = strlen(epubFilenameStr); i > 1 && epubFilenameStr[i] != '.'; i--)
			;
		strcpy(epubFilenameStr + i, ".epub");
		epubFilename = epubFilenameStr;
	}
	
	NEBegin(&ne, epubFilename);
	
	// alloc buffers
	src = malloc(SIZE);
	buf = malloc(SIZE);
	if (!src || !buf)
	{
		fprintf(stderr, "Not enough memory.\n");
		exit(1);
	}
	
	// process all files for adding XHTML parts
	outputFormat = NMEOutputFormatOPSXHTML;
	
	// add URL encoding fun to grab image references
	hookData.outputFormat = &outputFormat;
	hookData.ne = &ne;
	hookData.titleSrcOffset = 0;
	hookData.inImageMarkup = FALSE;
	outputFormat.encodeURLFun = encodeURL;
	outputFormat.encodeURLData = (void *)&hookData;
	outputFormat.spanHookFun = spanHookImg;
	outputFormat.hookData = (void *)&hookData;
	
	for (i = 0; ; i++)
		if (plugins[i].cb == PluginEndnote || plugins[i].cb == PluginMeta)
			plugins[i].userData = &ne;
		else if (plugins[i].cb == NULL)
			break;
	outputFormat.plugins = plugins;
	outputFormat.interwikis = interwikis;
	if (autoCCLink || autoURLLink)
	{
		int n = 0;
		if (autoCCLink)
			autoconverts[n++].cb = NMEAutoconvertCamelCase;
		if (autoURLLink)
			autoconverts[n++].cb = NMEAutoconvertURL;
		outputFormat.autoconverts = autoconverts;
	}
	for (i = iFiles; i < argc; i++)
	{
		// convert NME to XHTML
		fp = fopen(argv[i], "rb");
		if (!fp)
		{
			fprintf(stderr, "Cannot open file \"%s\".\n", argv[i]);
			exit(1);
		}
		srcLen = fread(src, 1, SIZE, fp);
		if (srcLen < 0)
		{
			free(src);
			exit(1);
		}
		fclose(fp);
		
		ne.currentDoc = argv[i];
		
		nmeerr = NMEProcess(src, srcLen,
				buf, SIZE,
				options, "\n", &outputFormat, 0,
				&dest, &destLen, NULL);
		
		if (nmeerr != kNMEErrOk)
		{
			fprintf(stderr, "Conversion error %d\n", nmeerr);
			exit(1);
		}
		
		// add converted file to epub
		NENewFile(&ne, argv[i]);
		NEWriteToFile(&ne, dest, destLen);
		NECloseFile(&ne);
		NEAddPart(&ne, argv[i], FALSE);
	}
	
	// add images
	imgFilename = NULL;
	for (;;)
	{
		char f[512];
		NEEnumOther(&ne, &imgFilename, &imgFilenameLength);
		if (!imgFilename)
			break;
		if (imgFilenameLength > 511)
			imgFilenameLength = 511;
		strncpy(f, imgFilename, imgFilenameLength);
		f[imgFilenameLength] = '\0';
		neerr = NEAddFile(&ne, f, f);
		if (neerr != kNEErrOk)
		{
			fprintf(stderr, "Cannot add image \"%s\"\n", f);
			exit(1);
		}
	}
	
	// process all parts for adding TOC entries
	outputFormat = NMEOutputFormatTextCompact;
	outputFormat.parHookFun = parHookTOC;
	outputFormat.hookData = (void *)&hookData;
	
	for (i = iFiles; i < argc; i++)
	{
		// convert NME to extract titles and add TOC entries
		fp = fopen(argv[i], "rb");
		if (!fp)
		{
			fprintf(stderr, "Cannot open file \"%s\".\n", argv[i]);
			exit(1);
		}
		srcLen = fread(src, 1, SIZE, fp);
		if (srcLen < 0)
		{
			free(src);
			exit(1);
		}
		fclose(fp);
		
		hookData.filename = argv[i];
		nmeerr = NMEProcess(src, srcLen,
				buf, SIZE,
				kNMEProcessOptDefault, "\n", &outputFormat, 0,
				&dest, &destLen, NULL);
		
		if (nmeerr != kNMEErrOk)
		{
			fprintf(stderr, "Conversion error %d\n", nmeerr);
			exit(1);
		}
	}
	
	NEMakeCover(&ne);
	
	NEEnd(&ne);
	
	// deallocate memory used for conversion
	free((void *)buf);
	free((void *)src);
	
	return 0;
}
