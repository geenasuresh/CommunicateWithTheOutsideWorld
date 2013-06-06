/**
 *	@file NE.h
 *	@brief Nyctergatis EPUB, a.k.a. NE (creation of EPUB documents).
 *	Relies on zlib (by Jean-Loup Gailly and Mark Adler) and minizip
 *  (by Gilles Vollant, distributed with zlib) for zip compression.
 *	@author Yves Piguet.
 *	@copyright 2010-2011, Yves Piguet.
 */

/**	@page nelicense NE License
 *
 *	Copyright (c) 2010-2011, Yves Piguet.
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions
 *	are met:
 *
Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

Neither the name of Yves Piguet nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	@section Remark Remarks
 *
 *	Sorry for shouting; we keep the new BSD license disclaimer verbatim.
 *
 *	The license above is known as the "new BSD license", i.e. without
 *	advertising clause. For a discussion on different open-source and
 *	"free" ("copyleft") licenses, please visit  http://www.opensource.org
 *	and http://www.gnu.org.
 */

/**	@page neusage NE Usage
 *
 *	@code
 *	NE ne;
 *	// begin EPUB and specify its path
 *	NEBegin(&ne, epubFilename);
 *	// add metadata (can do it at any time before NEEnd)
 *	NEAddMetadata(&ne, kNEMetaTitle, "...");
 *	...
 *	// add contents (XHTML, CSS, images, etc.) in any order, from files or in memory
 *	NEAddFile(&ne, filename, path);
 *	 and/or
 *	NENewFile(&ne, filename);
 *	NEWriteToFile(&ne, data, len);
 *	...
 *	NECloseFile(&ne);
 *	...
 *	// specify XHTML in the correct order
 *	NEAddPart(&ne, filename);
 *	...
 *	// specify other files in any order with their mime type
 *	NEAddOther(&ne, filename, mimetype);
 *	...
 *	// specify TOC entries in the correct order with hyperlinks to XHTML files
 *	NEAddTOCEntry(&ne, title, relativeUrl, level);
 *	...
 *	// finish the creation of the EPUB file
 *	NEEnd(&ne);
 *	@endcode
 */

#ifndef __NE__
#define __NE__

#include "zip.h"

typedef unsigned char NEBoolean;

#if !defined(NULL)
/// null pointer
#	define NULL 0
#endif

#if !defined(FALSE)
#	define FALSE 0	///< false boolean value
#	define TRUE 1	///< true boolean value
#endif

typedef enum
{
	kNEErrOk = 0,
	kNEErrCannotCreateEPUBFile,
	kNEErrZip,
	kNEErrMalloc,
	kNEErrCannotOpenFile,
} NEErr;

/// Kind of metadata
typedef enum
{
	kNEMetaTitle,	///< title
	kNEMetaCreator,	///< creator (author); may be multiple
	kNEMetaIdentifier,	///< unique identifier (ISBN, reversed domain name, etc.)
	kNEMetaLanguage,	///< language (RFC 3066; en, fr, de, ...)
	kNEMetaSubject,	///< subject; may be multiple
	kNEMetaDescription,	///< description
	kNEMetaPublisher,	///< publisher
	kNEMetaDate,	///< date (YYYY or YYYY-MM or YYYY-MM-DD)
	kNEMetaSource,	///< source
	kNEMetaRights	///< copyright notice
} NEMetadataKey;

/// Main NE structure (shouldn't be accessed directly)
typedef struct
{
	zipFile zf;	///< minizip reference
	
	// required metadata
	char *title;	///< book title
	char *creator;	///< book creator (lf-separated if multiple)
	char *identifier;	///< book identifier
	char *language;	// language (RFC 3066; lf-separated if multiple)
	
	// optional metadata
	char *subject;	///< subject (lf-separated if multiple)
	char *description;	///< description
	char *publisher;	///< publisher
	char *date;	///< date (YYYY or YYYY-MM or YYYY-MM-DD)
	char *source;	///< source
	char *rights;	///< copyright
	
	// endnotes
	char *endnotes;	///< XHTML document, built with NEAddEndnote
	int endnoteCount;	///< number of endnotes (last endnote label)
	char *lastRefLink;	///< last refLink created by NEAddEndnote
	char *currentDoc;	///< name of current document (not managed by NE)
	
	// parts (XHTML)
	char *parts;	///< main parts of the book (filenames, lf-separated)
	char *auxParts;	///< auxiliary parts of the book (filenames, linear="no", lf-separated)
	char *cover;	///< cover (filename)
	char *coverImage;	///< cover image (filename)
	
	// other documents (images, css, etc.)
	char *other;	///< filenames, lf-separated
	
	// toc entries
	char *tocEntries;	///< XML fragment (contents of navMap in NCX file)
	int ncxCount;	///< number of NCX entries
	int maxTOCDepth;	///< maximum toc depth
} NE, *NEPtr;

/**	Begin the creation of an EPUB file
	@param[out] ne reference to EPUB main structure
	@param[in] filename EPUB file name, or NULL for debug (write to stderr)
	@return kNEErrOk for success, error code for failure
*/
NEErr NEBegin(NEPtr ne, char const *filename);

/**	Add metadata
	@param[in,out] ne reference to EPUB main structure
	@param[in] key kind of metadata
	@param[in] data metadata value
	@param[in] dataLen length of data in bytes
	@return kNEErrOk for success, error code for failure
*/
NEErr NEAddMetadata(NEPtr ne,
	NEMetadataKey key, char const *data, int dataLen);

/**	Add an XHTML part to the book manifest; its contents should also be added
	with NEAddFile or NENewFile/NEWriteToFile/NECloseFile
	@param[in,out] ne reference to EPUB main structure
	@param[in] filename filename
	@param[in] auxiliary TRUE for auxiliary content (referenced via hyperlinks),
	FALSE for main contents (main line of reading)
	@return kNEErrOk for success, error code for failure
*/
NEErr NEAddPart(NEPtr ne, char const *filename, NEBoolean auxiliary);

/**	Add non-HTML content to the book (images, css, etc.); its contents should
	also be added with NEAddFile or NENewFile/NEWriteToFile/NECloseFile
	@param[in,out] ne reference to EPUB main structure
	@param[in] filename filename
	@param[in] filenameLen length of filename in bytes
	@param[in] mimetype MIME type (NULL to use filename's suffix)
	@return kNEErrOk for success, error code for failure
*/
NEErr NEAddOther(NEPtr ne,
		char const *filename, int filenameLen,
		char const *mimetype);

/**	Enumerate files added by NEAddOther
	@param[in] ne reference to EPUB main structure
	@param[in,out] filename previous filename on input (NULL to get the
	first one), next filename on output (NULL if no more)
	@param[out] filenameLength length of filename in bytes
*/
void NEEnumOther(NEPtr ne,
		char const **filename,
		int *filenameLength);

/**	Add an entry to the table of contents
	@param[in,out] ne reference to EPUB main structure
	@param[in] title entry text
	@param[in] relativeURL relative URL to the file with an optional anchor
	@param[in] level 1 for top-level entries, 2 for subentries, etc.
	@return kNEErrOk for success, error code for failure
*/
NEErr NEAddTOCEntry(NEPtr ne, char const *title, char const *relativeUrl, int level);

/**	Add the contents of a file
	@param[in,out] ne reference to EPUB main structure
	@param[in] filename file name as seen in the EPUB
	@param[in] path path of the original file
	@return kNEErrOk for success, error code for failure
*/
NEErr NEAddFile(NEPtr ne,
	char const *filename,
	char const *path);

/**	Create a new file in the EPUB
	@param[in,out] ne reference to EPUB main structure
	@param[in] filename filename in document subdirectory, or in EPUB root
	if it starts with "/"
	@return kNEErrOk for success, error code for failure
*/
NEErr NENewFile(NEPtr ne,
	char const *filename);

/**	Add data to the file created with NENewFile
	@param[in,out] ne reference to EPUB main structure
	@param[in] data data
	@param[in] len length of data in bytes
	@return kNEErrOk for success, error code for failure
*/
NEErr NEWriteToFile(NEPtr ne,
	char const *data, int len);

/**	Close the file opened by NENewFile
	@return kNEErrOk for success, error code for failure
*/
NEErr NECloseFile(NEPtr ne);

/**	Add an endnote.
	@param[in,out] ne reference to EPUB main structure
	@param[in] endnote XHTML code for the endnote (without number)
	@param[in] len length of endnote in bytes
	@param[in] refDoc name of the document containing the reference
	@param[in] refDocLen length of srcDoc in bytes
	@param[out] refLink XHTML code to use as a link to the endnote (null-terminated)
	@return kNEErrOk for success, error code for failure
*/
NEErr NEAddEndnote(NEPtr ne,
		char const *endnote, int len,
		char const *refDoc, int refDocLen,
		char const **refLink);

NEErr NEMakeCover(NEPtr ne);

/**	Specify an image to be used for the cover.
	@param[in,out] ne reference to EPUB main structure
	@param[in] filename filename of the cover image (should also be added
	with NENewFile or NEAddFile)
	@param[in] filenameLen length of filename in bytes, or -1 if null-terminated
	@return kNEErrOk for success, error code for failure
*/
NEErr NESetCoverImage(NEPtr ne, char const *filename, int filenameLen);

/**	Finish writing the EPUB file and release all resources
	@return kNEErrOk for success, error code for failure
*/
NEErr NEEnd(NEPtr ne);

// utility string functions

/**	Copy a string to a sring allocated by malloc.
	@param[in,out] str null-terminated string allocated by malloc, or NULL
	@param[in] src source string to be copied
	@param[in] srcLen length of src in bytes, or -1 if null-terminated
	@return kNEErrOk for success, error code for failure
*/
NEErr NEStringCopy(char **str, char const *src, int srcLen);

/**	Concatenate a string to a string allocated by malloc.
	@param[out] str null-terminated string allocated by malloc, or NULL
	@param[in] src source string to be concatenated
	@param[in] srcLen length of src in bytes, or -1 if null-terminated
	@return kNEErrOk for success, error code for failure
*/
NEErr NEStringCat(char **str, char const *src, int srcLen);

/**	Concatenate a string with a linefeed separator character.
	@param[out] str null-terminated string allocated by malloc, or NULL
	@param[in] src source string to be concatenated
	@param[in] srcLen length of src in bytes, or -1 if null-terminated
	@return kNEErrOk for success, error code for failure
*/
NEErr NEStringAdd(char **str, char const *src, int srcLen);

/**	Find next part after the next linefeed character
	@param[in] str input null-terminated string, or NULL
	@return address of string following the next (first) line-feed, or NULL if none
*/
char const *NEStringNextPart(char const *str);

/**	Deallocated string.
	@param[in,out] str string to be deallocated, or NULL
*/
void NEStringFree(char **str);


#endif
