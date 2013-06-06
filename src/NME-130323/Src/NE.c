/**
 *	@file NE.c
 *	@brief Nyctergatis EPUB (creation of EPUB documents).
 *	@author Yves Piguet.
 *	@copyright 2010-2011, Yves Piguet.
 */

/* License: new BSD license (see NE.h) */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zip.h"
#include "NE.h"

#if defined(_WIN32) || defined(_WIN64)
	/// Windows equivalent of strncasecmp
#	define strncasecmp _strnicmp
#endif

#define kBufferSize 65536L	// a larger value might be marginally more efficient
#define kPathSize 512	// must be large enough for document paths

#define DOCDIR "OPS"
#define ROOTDOC "content.opf"
#define NCXDOC "toc.ncx"
#define COVERDOC "cover.xhtml"
#define ENDNOTESDOC "endnotes.xhtml"

#define Chk(e) do { err = (e); if (err != kNEErrOk) return err; } while (0)

// string utility functions

NEErr NEStringCopy(char **str, char const *src, int srcLen)
{
	int i;
	
	if (*str)
		free(*str);
	
	if (srcLen < 0)
		srcLen = strlen(src);
	
	*str = malloc(srcLen + 1);
	if (!*str)
		return kNEErrMalloc;
	
	for (i = 0; i < srcLen; i++)
		(*str)[i] = src[i];
	(*str)[i] = '\0';
	
	return kNEErrOk;
}

NEErr NEStringCat(char **str, char const *src, int srcLen)
{
	if (!*str)
		return NEStringCopy(str, src, srcLen);
	else
	{
		int i;
		int strLen = strlen(*str);
		if (srcLen < 0)
			srcLen = strlen(src);
		
		*str = realloc(*str, strLen + srcLen + 1);
		if (!*str)
			return kNEErrMalloc;
		
		for (i = 0; i < srcLen; i++)
			(*str)[strLen + i] = src[i];
		(*str)[strLen + i] = '\0';
		return kNEErrOk;
	}
}

NEErr NEStringAdd(char **str, char const *src, int srcLen)
{
	if (!*str)
		return NEStringCopy(str, src, srcLen);
	else
	{
		int i;
		int strLen = strlen(*str);
		if (srcLen < 0)
			srcLen = strlen(src);
		
		*str = realloc(*str, strLen + srcLen + 2);
		if (!*str)
			return kNEErrMalloc;
		
		(*str)[strLen] = '\n';
		for (i = 0; i < srcLen; i++)
			(*str)[strLen + 1 + i] = src[i];
		(*str)[strLen + 1 + i] = '\0';
		return kNEErrOk;
	}
}

char const *NEStringNextPart(char const *str)
{
	if (!str)
		return NULL;
	while (str[0] && str[0] != '\n')
		str++;
	if (str[0])
		return str + 1;
	else
		return NULL;
}

int NEStringPartLength(char const *str)
{
	int n;
	for (n = 0; str[n] && str[n] != '\n'; n++)
		;
	return n;
}

NEBoolean NEStringEq(char const *str, char const *str2, int len2)
{
	int i;
	if (len2 >= 0)
	{
		for (i = 0; i < len2; i++)
			if (str[i] != str2[i])
				return FALSE;
	}
	else
		for (i = 0; str2[i] && str2[i] != '\n'; i++)
			if (str[i] != str2[i])
				return FALSE;
	return str[i] == '\0' || str[i] == '\n';
}

void NEStringFree(char **str)
{
	if (*str)
		free(*str);
	*str = NULL;
}

NEErr NEBegin(NEPtr ne, char const *filename)
{
	int zerr;
	static char const mimetype[] = "application/epub+zip\n";
	
	if (filename)
	{
		ne->zf = zipOpen64(filename, APPEND_STATUS_CREATE);
		if (!ne->zf)
			return kNEErrCannotCreateEPUBFile;
	}
	else
		ne->zf = NULL;
	
	// initialize meta information
	ne->title = NULL;
	NEStringCopy(&ne->title, "Untitled", -1);
	ne->creator = NULL;
	ne->identifier = NULL;
	NEStringCopy(&ne->identifier, "no-identifier", -1);
	ne->language = NULL;
	ne->subject = NULL;
	ne->description = NULL;
	ne->publisher = NULL;
	ne->date = NULL;
	ne->source = NULL;
	ne->rights = NULL;
	
	// initialize parts and other documents
	ne->parts = NULL;
	ne->auxParts = NULL;
	ne->cover = NULL;
	ne->other = NULL;
	ne->coverImage = NULL;
	
	// initialize other fields
	ne->tocEntries = NULL;
	ne->ncxCount = 0;
	ne->maxTOCDepth = 1;
	ne->endnotes = NULL;
	ne->endnoteCount = 0;
	ne->lastRefLink = NULL;
	ne->currentDoc = NULL;
	
	// add file "mimetype" (first file, uncompressed)
	zerr = zipOpenNewFileInZip(ne->zf,
			"mimetype", NULL,
			NULL, 0, NULL, 0,
			NULL,
			0, 0);
	if (zerr != Z_OK)
		return kNEErrZip;
	zerr = zipWriteInFileInZip(ne->zf, mimetype, strlen(mimetype));
	if (zerr != Z_OK)
		return kNEErrZip;
	zerr = zipCloseFileInZip(ne->zf);
	if (zerr != Z_OK)
		return kNEErrZip;
	
	return kNEErrOk;
}

NEErr NEAddMetadata(NEPtr ne,
	NEMetadataKey key, char const *data, int dataLen)
{
	switch (key)
	{
		case kNEMetaTitle:
			NEStringCopy(&ne->title, data, dataLen);
			break;
		case kNEMetaCreator:
			NEStringAdd(&ne->creator, data, dataLen);
			break;
		case kNEMetaIdentifier:
			NEStringCopy(&ne->identifier, data, dataLen);
			break;
		case kNEMetaLanguage:
			NEStringAdd(&ne->language, data, dataLen);
			break;
		case kNEMetaSubject:
			NEStringAdd(&ne->subject, data, dataLen);
			break;
		case kNEMetaDescription:
			NEStringCopy(&ne->description, data, dataLen);
			break;
		case kNEMetaPublisher:
			NEStringCopy(&ne->publisher, data, dataLen);
			break;
		case kNEMetaDate:
			NEStringCopy(&ne->date, data, dataLen);
			break;
		case kNEMetaSource:
			NEStringCopy(&ne->source, data, dataLen);
			break;
		case kNEMetaRights:
			NEStringCopy(&ne->rights, data, dataLen);
			break;
	}
	return kNEErrOk;
}

NEErr NESetCoverImage(NEPtr ne, char const *filename, int filenameLen)
{
	NEErr err;
	//Chk(NEStringAdd(&ne->auxParts, filename, filenameLen));
	Chk(NEStringCopy(&ne->coverImage, filename, filenameLen));
	Chk(NEAddFile(ne, ne->coverImage, ne->coverImage));
	return kNEErrOk;
}

NEErr NEAddPart(NEPtr ne, char const *filename, NEBoolean auxiliary)
{
	NEErr err;
	
	if (auxiliary)
		Chk(NEStringAdd(&ne->auxParts, filename, -1));
	else
		Chk(NEStringAdd(&ne->parts, filename, -1));
	return kNEErrOk;
}

static char const *SuffixToMimetype(char const *filename, int filenameLen)
{
	int i, suffix;
	static struct
	{
		char const *suffix;
		char const *mimetype;
	} const m[] = {
		{"gif", "image/gif"},
		{"jpg", "image/jpeg"},
		{"png", "image/png"},
		{"svg", "image/svg+xml"},
		{"xhtml", "text/xhtml+xml"},
		{"css", "text/css"},
		{"xml", "application/xml"},
		{"ncx", "application/x-dtbncx+xml"},
		{NULL, NULL}
	};
	
	if (filenameLen < 0)
		filenameLen = strlen(filename);
	for (suffix = filenameLen; suffix > 0 && filename[suffix - 1] != '.'; suffix--)
		;
	if (suffix <= 0)
		return "text/plain";	// default
	
	for (i = 0; m[i].suffix; i++)
		if (!strncasecmp(filename + suffix, m[i].suffix, filenameLen - suffix))
			return m[i].mimetype;
	
	return "text/plain";	// default
}

NEErr NEAddOther(NEPtr ne,
		char const *filename, int filenameLen,
		char const *mimetype)
{
	NEStringAdd(&ne->other, filename, filenameLen);
	NEStringAdd(&ne->other,
			mimetype ? mimetype : SuffixToMimetype(filename, filenameLen),
			-1);
	return kNEErrOk;
}

void NEEnumOther(NEPtr ne,
		char const **filename,
		int *filenameLength)
{
	if (*filename)
	{
		*filename = NEStringNextPart(*filename);	// skip mimetype
		*filename = NEStringNextPart(*filename);
	}
	else
		*filename = ne->other;
	if (*filename)
		*filenameLength = NEStringPartLength(*filename);
}

NEErr NEAddFile(NEPtr ne,
	char const *filename,
	char const *path)
{
	FILE *fp = NULL;
	char *buffer = NULL;
	long n;
	NEErr err;
	
	if (!ne->zf)
		return kNEErrOk;
	
	buffer = malloc(kBufferSize);
	if (!buffer)
		return kNEErrMalloc;
	fp = fopen(path, "rb");
	if (!fp)
	{
		free(buffer);
		return kNEErrCannotOpenFile;
	}
	
	err = NENewFile(ne, filename);
	if (err != kNEErrOk)
		goto error;
	
	for (;;)
	{
		n = fread(buffer, 1, kBufferSize, fp);
		if (n <= 0)
			break;
		err = NEWriteToFile(ne, buffer, n);
		if (err != kNEErrOk)
		{
			NECloseFile(ne);
			goto error;
		}
	}
	
	free(buffer);
	NECloseFile(ne);
	return kNEErrOk;
	
error:
	if (fp)
		fclose(fp);
	if (buffer)
		free(buffer);
	return err;
}

NEErr NEAddTOCEntry(NEPtr ne, char const *title, char const *relativeUrl, int level)
{
	char str[16];
	
	ne->ncxCount++;
	if (level > ne->maxTOCDepth)
		ne->maxTOCDepth = level;
	
	// warning: level isn't used yet
	
	NEStringCat(&ne->tocEntries, "<navPoint id=\"", -1);
	sprintf(str, "p%d", ne->ncxCount);
	NEStringCat(&ne->tocEntries, str, -1);
	NEStringCat(&ne->tocEntries, "\" playOrder=\"", -1);
	sprintf(str, "%d", ne->ncxCount);
	NEStringCat(&ne->tocEntries, str, -1);
	NEStringCat(&ne->tocEntries, "\"><navLabel><text>", -1);
	NEStringCat(&ne->tocEntries, title, -1);
	NEStringCat(&ne->tocEntries, "</text></navLabel><content src=\"", -1);
	NEStringCat(&ne->tocEntries, relativeUrl, -1);
	NEStringCat(&ne->tocEntries, "\"/></navPoint>\n", -1);
	
	return kNEErrOk;
}

NEErr NENewFile(NEPtr ne,
	char const *filename)
{
	int zerr;
	char filename2[kPathSize];
	
	if (!ne->zf)
		return kNEErrOk;
	
	if (filename[0] == '/')
	{
		// skip slash
		strncpy(filename2, filename + 1, kPathSize);
	}
	else
	{
		// prepend DOCDIR "/"
		strcpy(filename2, DOCDIR "/");
		strncat(filename2, filename, kPathSize);
	}
	
	zerr = zipOpenNewFileInZip(ne->zf,
			filename2, NULL,
			NULL, 0, NULL, 0,
			NULL,
			Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	return zerr == Z_OK ? kNEErrOk : kNEErrZip;
}

NEErr NEWriteToFile(NEPtr ne,
	char const *data, int len)
{
	if (!ne->zf)
	{
		fwrite(data, 1, len >= 0 ? len : strlen(data), stdout); 
		return kNEErrOk;
	}
	
	if (len < 0)
		len = strlen(data);
	return zipWriteInFileInZip(ne->zf, data, len) == Z_OK ? kNEErrOk : kNEErrZip;
}

NEErr NECloseFile(NEPtr ne)
{
	if (!ne->zf)
		return kNEErrOk;
	return zipCloseFileInZip(ne->zf) == Z_OK ? kNEErrOk : kNEErrZip;
}

NEErr NEAddEndnote(NEPtr ne,
		char const *endnote, int len,
		char const *refDoc, int refDocLen,
		char const **refLink)
{
	char str[16];
	NEBoolean beginsWithP, quoted, squoted;
	int i;
	
	sprintf(str, "%d", ++ne->endnoteCount);
	
	// create refLink = &nbsp;<a href="ENDNOTESDOC#enN" id="enRefN">[N]</a>
	NEStringCopy(&ne->lastRefLink, "&nbsp;<a href=\"" ENDNOTESDOC "#en", -1);
	NEStringCat(&ne->lastRefLink, str, -1);
	NEStringCat(&ne->lastRefLink, "\" id=\"enRef", -1);
	NEStringCat(&ne->lastRefLink, str, -1);
	NEStringCat(&ne->lastRefLink, "\">[", -1);
	NEStringCat(&ne->lastRefLink, str, -1);
	NEStringCat(&ne->lastRefLink, "]</a>", -1);
	*refLink = ne->lastRefLink;
	
	// create endnote
	// <div class="endnote" id="enN"><p><a href="refDoc#enRefN">[N]</a> endnote</p></div>
	// insert hyperlink at the beginning of first p element or prepend a new
	//  p element with the hyperlink alone if it is another type (typically pre)
	NEStringCat(&ne->endnotes, "<div class=\"endnote\" id=\"en", -1);
	NEStringCat(&ne->endnotes, str, -1);
	NEStringCat(&ne->endnotes, "\">\n", -1);
	beginsWithP = endnote[0] == '<' && endnote[1] == 'p'
		&& (endnote[2] < 'a' || endnote[2] > 'z')
		&& (endnote[2] < 'A' || endnote[2] > 'Z');
	if (beginsWithP)
	{
		// find complete p tag (including attributes) until 
		for (i = 0, quoted = squoted = FALSE; i < len; i++)
			if (!squoted && endnote[i] == '"')
				quoted = !quoted;
			else if (!quoted && endnote[i] == '\'')
				squoted = !squoted;
			else if (!quoted && !squoted && endnote[i] == '>')
				break;
		if (i < len)
			i++;
		// copy it to ne->endnotes
		NEStringCat(&ne->endnotes, endnote, i);
	}
	else
		NEStringCat(&ne->endnotes, "<p>", -1);
	// link
	NEStringCat(&ne->endnotes, "<a href=\"", -1);
	NEStringCat(&ne->endnotes, refDoc, refDocLen);
	NEStringCat(&ne->endnotes, "#enRef", -1);
	NEStringCat(&ne->endnotes, str, -1);
	NEStringCat(&ne->endnotes, "\">[", -1);
	NEStringCat(&ne->endnotes, str, -1);
	NEStringCat(&ne->endnotes, "]</a>", -1);
	if (beginsWithP)
	{
		NEStringCat(&ne->endnotes, " ", -1);
		NEStringCat(&ne->endnotes, endnote + i, len - i);
	}
	else
	{
		NEStringCat(&ne->endnotes, "</p>\n", -1);
		NEStringCat(&ne->endnotes, endnote, len);
	}
	NEStringCat(&ne->endnotes, "</div>\n", -1);
	
	return kNEErrOk;
}

NEErr NEMakeCover(NEPtr ne)
{
	NEErr err;
	
	Chk(NENewFile(ne, COVERDOC));
	Chk(NEWriteToFile(ne,
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<!DOCTYPE html PUBLIC\n"
		" \"-//W3C//DTD XHTML 1.1//EN\"\n"
		" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"<head>\n"
		"<title>Cover</title>\n"
		"<style type=\"text/css\">img {max-width: 100%;}</style>\n"
		"</head>\n"
		"<body class=\"cover\">\n"
		, -1));
	if (ne->coverImage)
	{
		Chk(NEWriteToFile(ne, "<div><img src=\"", -1));
		Chk(NEWriteToFile(ne, ne->coverImage, -1));
		Chk(NEWriteToFile(ne, "\" alt=\"Cover\"/></div>", -1));
	}
	else
	{
		Chk(NEWriteToFile(ne,
			"<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"\n"
			" xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
			" width=\"100%\" height=\"100%\" viewBox=\"0 0 500 656\">\n"
			"<rect x=\"0\" y=\"0\" fill=\"blue\" width=\"500\" height=\"656\"/>\n"
			, -1));
		if (ne->title)
		{
			Chk(NEWriteToFile(ne,
				"<text font-family=\"Helvetica\" font-size=\"36\" fill=\"white\" x=\"100\" y=\"350\">"
				, -1));
			Chk(NEWriteToFile(ne, ne->title, -1));
			Chk(NEWriteToFile(ne,
				"</text>\n"
				, -1));
		}
		if (ne->creator)
		{
			Chk(NEWriteToFile(ne,
				"<text font-family=\"Helvetica\" font-size=\"24\" fill=\"white\" x=\"100\" y=\"250\">"
				, -1));
			Chk(NEWriteToFile(ne, ne->creator, -1));
			Chk(NEWriteToFile(ne,
				"</text>\n"
				, -1));
		}
		Chk(NEWriteToFile(ne,
			"</svg>\n"
			, -1));
	}
	Chk(NEWriteToFile(ne,
		"</body>\n"
		"</html>\n"
		, -1));
	Chk(NECloseFile(ne));
	Chk(NEStringCopy(&ne->cover, COVERDOC, -1));
	return kNEErrOk;
}

/**	Write document metadata.
	@param[in,out] ne reference to EPUB main structure
	@param[in] name metadata name in the dc namespace (e.g. "title")
	@param[in] attr if not NULL, attributes ("n1=\"v1\" n2=\"v2\" etc.",
	where names have the proper namespace)
	@param[in] str metadata value
	@return kNEErrOk for success, error code for failure
*/	
static NEErr WriteMetadata(NEPtr ne,
	char const *name, char const *attr, char const *str)
{
	char const *sub;
	
	for (sub = str; sub; sub = NEStringNextPart(sub))
	{
		NEErr err;
		
		Chk(NEWriteToFile(ne, "  <dc:", -1));
		Chk(NEWriteToFile(ne, name, -1));
		if (attr)
		{
			Chk(NEWriteToFile(ne, " ", -1));
			Chk(NEWriteToFile(ne, attr, -1));
		}
		Chk(NEWriteToFile(ne, ">", -1));
		Chk(NEWriteToFile(ne, sub, NEStringPartLength(sub)));
		Chk(NEWriteToFile(ne, "</dc:", -1));
		Chk(NEWriteToFile(ne, name, -1));
		Chk(NEWriteToFile(ne, ">\n", -1));
	}
	return kNEErrOk;
}

/**	Write a meta element.
	@param[in,out] ne reference to EPUB main structure
	@param[in] name name
	@param[in] content content
	@return kNEErrOk for success, error code for failure
*/
static NEErr WriteMeta(NEPtr ne,
	char const *name, char const *content)
{
	NEErr err;
	
	Chk(NEWriteToFile(ne, "  <meta name=\"", -1));
	Chk(NEWriteToFile(ne, name, -1));
	Chk(NEWriteToFile(ne, "\" content=\"", -1));
	Chk(NEWriteToFile(ne, content, -1));
	Chk(NEWriteToFile(ne, "\"/>\n", -1));
	
	return kNEErrOk;
}

/**	Write the file for endnotes.
	@param[in,out] ne reference to EPUB main structure
	@return kNEErrOk for success, error code for failure
*/
static NEErr WriteEndnotes(NEPtr ne)
{
	NEErr err;
	
	if (!ne->endnotes)
		return kNEErrOk;
	
	Chk(NENewFile(ne, ENDNOTESDOC));
	Chk(NEWriteToFile(ne,
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<!DOCTYPE html PUBLIC\n"
		" \"-//W3C//DTD XHTML 1.1//EN\"\n"
		" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"<head>\n"
		"<title>End Notes</title>\n"
		"<style type=\"text/css\">\n"
		"div.endnote {page-break-before: always}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n",
		-1));
	Chk(NEWriteToFile(ne,
		ne->endnotes,
		-1));
	Chk(NEWriteToFile(ne,
		"</body>\n"
		"</html>\n",
		-1));
	Chk(NECloseFile(ne));
	Chk(NEAddPart(ne, ENDNOTESDOC, TRUE));
	
	return kNEErrOk;
}

/**	Write the OPF file.
	@param[in,out] ne reference to EPUB main structure
	@return kNEErrOk for success, error code for failure
*/
static NEErr WriteOPF(NEPtr ne)
{
	int p;
	NEErr err;
	char const *sub;
	int id;
	char idStr[16];	// "idN" where N is 1, 2, ...
	
	Chk(NENewFile(ne, ROOTDOC));
	Chk(NEWriteToFile(ne,
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<package version=\"2.0\"\n"
		" unique-identifier=\"PrimaryID\"\n"
		" xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
		" xmlns=\"http://www.idpf.org/2007/opf\">\n"
		" <metadata>\n",
		-1));
	
	Chk(NEWriteToFile(ne, "  <dc:identifier id=\"PrimaryID\">", -1));
	Chk(NEWriteToFile(ne, ne->identifier, -1));
	Chk(NEWriteToFile(ne, "</dc:identifier>\n", -1));
	Chk(WriteMetadata(ne, "title", NULL, ne->title));
	Chk(WriteMetadata(ne, "creator", NULL /* "opf:role=\"aut\"" */, ne->creator));
	Chk(WriteMetadata(ne, "subject", NULL, ne->subject));
	Chk(WriteMetadata(ne, "description", NULL, ne->description));
	Chk(WriteMetadata(ne, "publisher", NULL, ne->publisher));
	Chk(WriteMetadata(ne, "date", NULL, ne->date));
	Chk(WriteMetadata(ne, "source", NULL, ne->source));
	Chk(WriteMetadata(ne, "language", NULL, ne->language ? ne->language : "en"));
	Chk(WriteMetadata(ne, "rights", NULL, ne->rights));
	Chk(WriteMeta(ne, "cover", "CoverID"));
	
	Chk(NEWriteToFile(ne,
		" </metadata>\n"
		" <manifest>\n",
		-1));
	Chk(NEWriteToFile(ne,
		"  <item id=\"ncx\"\n   href=\""
		NCXDOC
		"\"\n   media-type=\"application/x-dtbncx+xml\"/>\n",
		-1));
	for (p = 0, id = 1; p < 3; p++)
		for (sub = p == 0 ? ne->cover : p == 1 ? ne->parts : ne->auxParts;
			sub;
			sub = NEStringNextPart(sub), id++)
		{
			Chk(NEWriteToFile(ne, "  <item id=\"", -1));
			sprintf(idStr, "id%d", id);
			Chk(NEWriteToFile(ne, idStr, -1));
			Chk(NEWriteToFile(ne, "\"\n   href=\"", -1));
			Chk(NEWriteToFile(ne, sub, NEStringPartLength(sub)));
			Chk(NEWriteToFile(ne, "\"\n   media-type=\"application/xhtml+xml\"/>\n", -1));
		}
	for (sub = ne->other; sub; sub = NEStringNextPart(sub), id++)
	{
		Chk(NEWriteToFile(ne, "  <item id=\"", -1));
		sprintf(idStr, "id%d", id);
		Chk(NEWriteToFile(ne, idStr, -1));
		Chk(NEWriteToFile(ne, "\"\n   href=\"", -1));
		Chk(NEWriteToFile(ne, sub, NEStringPartLength(sub)));
		sub = NEStringNextPart(sub);
		Chk(NEWriteToFile(ne, "\"\n   media-type=\"", -1));
		Chk(NEWriteToFile(ne, sub, NEStringPartLength(sub)));
		Chk(NEWriteToFile(ne, "\"/>\n", -1));
	}
	if (ne->coverImage)
	{
		Chk(NEWriteToFile(ne, "  <item id=\"CoverID\"\n   href=\"", -1));
		Chk(NEWriteToFile(ne, ne->coverImage, -1));
		Chk(NEWriteToFile(ne, "\"\n   media-type=\"", -1));
		Chk(NEWriteToFile(ne, SuffixToMimetype(ne->coverImage, -1), -1));
		Chk(NEWriteToFile(ne, "\"/>\n", -1));
	}
	Chk(NEWriteToFile(ne,
		" </manifest>\n"
		" <spine toc=\"ncx\">\n",
		-1));
	for (p = 0, id = 1; p < 3; p++)
		for (sub = p == 0 ? ne->cover : p == 1 ? ne->parts : ne->auxParts;
			sub;
			sub = NEStringNextPart(sub), id++)
		{
			Chk(NEWriteToFile(ne, " <itemref idref=\"", -1));
			sprintf(idStr, "id%d", id);
			Chk(NEWriteToFile(ne, idStr, -1));
			Chk(NEWriteToFile(ne,
					p == 1 ? "\"/>\n" : "\" linear=\"no\"/>\n", -1));
		}
	Chk(NEWriteToFile(ne,
		" </spine>\n",
		-1));
	if (ne->cover)
	{
		Chk(NEWriteToFile(ne,
			" <guide>\n"
			"  <reference type=\"cover\" title=\"Cover\" href=\"",
			-1));
		Chk(NEWriteToFile(ne, ne->cover, -1));
		Chk(NEWriteToFile(ne,
			"\"/>\n"
			" </guide>\n",
			-1));
	}
	Chk(NEWriteToFile(ne,
		"</package>\n",
		-1));
	Chk(NECloseFile(ne));
	
	return kNEErrOk;
}

/**	Write the NCX file.
	@param[in,out] ne reference to EPUB main structure
	@return kNEErrOk for success, error code for failure
*/
static NEErr WriteNCX(NEPtr ne)
{
	NEErr err;
	char str[16];
	
	Chk(NENewFile(ne, NCXDOC));
	Chk(NEWriteToFile(ne,
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<!DOCTYPE ncx PUBLIC \"-//NISO//DTD ncx 2005-1//EN\" "
			"\"http://www.daisy.org/z3986/2005/ncx-2005-1.dtd\">\n"
		"<ncx version=\"2005-1\" xml:lang=\"en-US\"\n"
		" xmlns=\"http://www.daisy.org/z3986/2005/ncx/\">\n",
		-1));
	Chk(NEWriteToFile(ne,
		" <head>\n",
		-1));
	Chk(NEWriteToFile(ne,
		"  <meta name=\"dtb:uid\" content=\"",
		-1));
	Chk(NEWriteToFile(ne, ne->identifier, -1));
	Chk(NEWriteToFile(ne,
		"\"/>\n",
		-1));
	sprintf(str, "%d", ne->maxTOCDepth);
	Chk(NEWriteToFile(ne,
		"  <meta name=\"dtb:depth\" content=\"",
		-1));
	Chk(NEWriteToFile(ne, str, -1));
	Chk(NEWriteToFile(ne,
		"\"/>\n",
		-1));
	Chk(NEWriteToFile(ne,
		"  <meta name=\"dtb:totalPageCount\" content=\"0\"/>\n",
		-1));
	Chk(NEWriteToFile(ne,
		"  <meta name=\"dtb:maxNumberPage\" content=\"0\"/>\n",
		-1));
	Chk(NEWriteToFile(ne,
		" </head>\n",
		-1));
	Chk(NEWriteToFile(ne,
		" <docTitle><text>",
		-1));
	Chk(NEWriteToFile(ne, ne->title ? ne->title : "Untitled", -1));
	Chk(NEWriteToFile(ne,
		"</text></docTitle>\n",
		-1));
	Chk(NEWriteToFile(ne,
		" <navMap>\n",
		-1));
	if (ne->tocEntries)
		Chk(NEWriteToFile(ne, ne->tocEntries, -1));
	Chk(NEWriteToFile(ne,
		" </navMap>\n",
		-1));
	Chk(NEWriteToFile(ne,
		"</ncx>\n",
		-1));
	Chk(NECloseFile(ne));
	
	return kNEErrOk;
}

NEErr NEEnd(NEPtr ne)
{
	int zerr;
	NEErr err;
	
	Chk(WriteEndnotes(ne));
	Chk(WriteOPF(ne));
	Chk(WriteNCX(ne));
	
	// write META-INF/container.xml
	Chk(NENewFile(ne, "/META-INF/container.xml"));
	Chk(NEWriteToFile(ne,
		"<?xml version=\"1.0\"?>\n"
		"<container version=\"1.0\" "
		" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
		" <rootfiles>\n"
		"  <rootfile full-path=\"" DOCDIR "/" ROOTDOC "\""
		"   media-type=\"application/oebps-package+xml\"/>\n"
		" </rootfiles>\n"
		"</container>\n"
		, -1));
	Chk(NECloseFile(ne));
	
	zerr = zipClose(ne->zf, NULL);
	
	// release all resources
	NEStringFree(&ne->title);
	NEStringFree(&ne->creator);
	NEStringFree(&ne->identifier);
	NEStringFree(&ne->language);
	NEStringFree(&ne->subject);
	NEStringFree(&ne->description);
	NEStringFree(&ne->publisher);
	NEStringFree(&ne->date);
	NEStringFree(&ne->source);
	NEStringFree(&ne->rights);
	NEStringFree(&ne->endnotes);
	NEStringFree(&ne->lastRefLink);
	NEStringFree(&ne->parts);
	NEStringFree(&ne->auxParts);
	NEStringFree(&ne->cover);
	NEStringFree(&ne->coverImage);
	NEStringFree(&ne->other);
	NEStringFree(&ne->tocEntries);
	
	if (zerr != Z_OK)
		return kNEErrZip;
	return kNEErrOk;
}
