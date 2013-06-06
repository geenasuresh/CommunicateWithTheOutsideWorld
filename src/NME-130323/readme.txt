   Nyctergatis Markup Engine

Nyctergatis Markup Engine (NME) is a library for parsing text with
markup and converting it to other text formats. The markup has the
following goals:

 - A set of simple rules a user can learn and rely on.
 - Robust specifications with room for evolution without breaking
   compatibility.

The markup is based on Creole. Creole is a collaborative work in
progress; only its core features are frozen. This is why the
documentation of the markup used by NME is not Creole itself, but the
documentation provided with the engine.

NME provides the core cross-platform conversion code and optional
extensions which can be embedded in other applications;
platform-specific support functions for C++, Windows MFC, OS X
(Objective-C), and GTK+ (Linux); and command-line tools for
single-file conversion as well as the creation of EPUB files from a
set of files.

  Overview

The library core is made of two source files in C:

NME.h
  header file with all extern declarations and detailed usage
   information, compatible with C++;
NME.c
  function and data definitions.

In addition, the following optional code is provided:

NMEStyle.h and NMEStyle.c
  functions to collect style information separately
NMEAutolink.h and NMEAutolink.c
  automatic conversion of CamelCase words and URLs to links;
NMEPluginCalendar.h and NMEPluginCalendar.c
  plugin to create the calendar corresponding to a given month and
   year in a table.
NMEPluginRaw.h and NMEPluginRaw.c
  plugin to embed raw data copied verbatim to the output stream;
NMEPluginReverse.h and NMEPluginReverse.c
  plugin to reverse text;
NMEPluginRot13.h and NMEPluginRot13.c
  plugin to apply ROT-13 conversion to text (letters A-M and N-Z
   are switched);
NMEPluginTOC.h and NMEPluginTOC.c
  plugin to create a table of contents;
NMEPluginUppercase.h and NMEPluginUppercase.c
  plugin to convert text to uppercase;
NMEPluginWiki.h and NMEPluginWiki.c
  plugin to extract metadata and title;
NMETest.h and NMETest.c
  implementation test (check the correct use of output markup and
   hooks);
NMECpp.h, NMEErrorCpp.h and NMEStyleCpp.h
  C++ glue;
NMEGtk.h and NMEGtk.c
  display of text with NME markup as styled text in GTK+ (Linux
   and Unix);
NMEMFC.h and NMEMFC.cpp
  display of text with NME markup as styled text in MFC (Windows);
NMEObjC.h and NMEObjC.m
  display of text with NME markup as styled text in Cocoa (OS X).

These files implement applications:

NMEMain.c
  command-line application which filters its input (stdin), which
   is text with markup, and writes to its output (stdout) the result
   of the conversion to HTML or several other formats. To build it,
   cd to the NME distribution directory and type make;
NMERandomGen.c
  command-line tool which generates random input to test NME. To
   build it, cd to the NME distribution directory and type make
   nmerandom;
NMETest.cpp
  simple C++ test application. To build it, cd to the NME
   distribution directory and type make nmecpp;
NMEGtkTest.c
  test application for NMEGtk which displays styled text in a GTK
   window. It requires GTK+ 2.0, available at http://www.gtk.org. To
   build it, cd to the NME distribution directory and type make
   nmegtk.

NME can also be used to create EPUB files. EPUB is a popular
format for e-books (see below). Here are the files related to EPUB:

NE.h and NE.c
  EPUB file building, with contents provided as XHTML (independent
   of NME, relies on the library zlib for zip creation);
NMEEPub.h and NMEEPub.c
  NME style to convert from NME markup to EPUB XHTML;
NMEEPubMain.c
  command-line application which creates EPUB files from a set of
   text files with NME markup.

  EPUB

EPUB is a "distribution and interchange format standard for digital
publications and documents". It is a standard designed by the 
International Digital Publishing Forum (IDPF) and used by many e-book
readers, such as iBook on Apple iPad and other IOS devices, and
software applications on most platforms.

EPUB are zip files with a .epub extension. They contain a set of
content files in xhtml and other formats (jpeg, svg, etc. for
images), and additional files for the document structure and
information about the title, author, and other metadata.

 Command-line tool

The command-line tool nmeepub, whose source code is provided with
NME, converts a set of NME files to a single EPUB document.
Typically, the book is written as a single NME file or as one NME
file per chapter. Chapters start with a title (line starting with =),
sections (if any) with a subtitle (line starting with ==), etc. A
table of contents is created automatically.

Images can be embedded with {{...}}. In addition, the following
extensions are implemented:

Text<< endnote Text of the endnote >>

Notes are collected and linked forward and backward to the text they
are related to. The endnote text itself can contain NME markup.

<< title Document title >>
<< author Author name >>
<< identifier Document identifier >>
<< language Language ISO code >>
<< subject Document subject >>
<< description Document description >>
<< publisher Publisher name >>
<< date Publishing date (ISO format) >>
<< source Original source of the document >>
<< rights Copyright notice >>

Metadata information can be placed anywhere in the NME files.

<< cover Cover image file >>

The cover image file (e.g. cover.png) is an image to be used for the
book cover. Currently, images are the only reliable way to have the
desired cover on all platforms, and must be created outside nmeepub.

 Usage

With nmeepub, it is extremely simple to convert a text document to a
high-quality, easy-to-read e-book which can be distributed or just
copied to your reader for personal use. Starting from the text of a
book, for instance, you can open it in a text editor and quickly find
titles to mark them with an equal sign, and throw some metadata tags
(at least the title and author to identify quickly the document in
your virtual library). Then once the conversion is done, the
resulting EPUB file can be transferred to your reading device (for
the iPad, drag the file to iTunes and synchronize your tablet).

The next step is to add a cover. Use any image editing application,
create an image (typical dimension: height of 780 pixels, width of
580 pixels), save it as a PNG file (e.g. cover.png in the same folder
as your NME files), and add << cover cover.png >> in your NME file.

A typical NME file could look like this:

<< title My Book >>
<< author My Name >>
<< cover cover.png >>

= Chapter 1
...

= Chapter 2
...

  License

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of Yves Piguet nor the names of its contributors may
be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------

  Release notes

 23 March 2013

 - Parenthesis in boolean expressions to suppress gcc warnings
 - Documentation of NME usage updated
 - Global XML entity dictionary NMEXMLCharDict
 - Links used verbatim as link text when no explicit link text is
   provided
 - Buffer overflow in addLinkBegin fixed
 - %L in beginDD fixed

 23 October 2011

 - Superfluous scoping operator for copy operator removed
 - NMETest.cpp renamed to avoid object file name collision
 - NMETest.c and NMETest.h added to distrib in Makefile

 4 October 2011

 - EPUB support
 - support for RichEdit version >= 0x0200 on MFC
 - tracking of NME source line number
 - conditional expressions in output format strings with ?:
 - }}} handled as normal characters outside pre block
 - bug fixes related to nesting of output markup and hook calls
 - tests with new nmerandom tool
 - Visual C++ 2010 projects and code adjustments
 - improved Doxygen documentation
 - with LaTeX output, numbered and definition lists are translated
   to enumerate and definition environments respectively instead of
   itemize
 - gcc warning hex escape sequence out of range fixed in
   NMEPluginWiki.h
 - new functions NMEResetOutput and NMECurrentInputIndex
 - list following paragraphs without empty line resulted in
   invalid HTML; fixed

 31 March 2010

 - arrays in lists didn't balance closing tags correctly; fixed
   (arrays force list end)
 - sublists weren't converted to compliant HTML (they were outside
   <li></li> markup); fixed
 - the distribution archive lacked NMEStyle.c, NMEStyle.h and
   NMEErrorCpp.h; fixed
 - make distrib didn't work with what it produced; fixed
 - wrong description of function NMEPluginTOC; fixed
 - --null-plugins option in NMEMain.c
 - NMEPluginWiki.h, NMEPluginWiki.c and option --metadata in
   NMEMain.c (metadata and title extraction)
 - use id attributes for anchors instead of name in a separate a
   element
 - if useHTMLEmphasisTags is defined, use <em></em> and
   <strong></strong> instead of <i></i> and <b></b> respectively
 - }{ was wrongly recognized as an image opening tag; fixed
 - NMECurrentListNesting uses | for tables
 - context is available in wordwrapPermFun callback and used for
   NME output to prevent line breaks in tables
 - indentation wasn't always set correctly, resulting in too many
   leading spaces at the beginning of lines; fixed
 - new NMEOutputFormatDebug format for debugging, and --debug and
   --debug2 options in NMEMain.c
 - new --structspan option in NMEMain.c
 - Objective-C wrapper (files NMEObjC.h and NMEObjC.m)

 19 November 2008

 - new hook function charHookFun called when a character is copied to
   output; useful to match text fragments in input and output
 - new hook function getVarFun for custom variables in expressions
 - character = was skipped in headings even when it wasn't the
   optional trailing heading markup; fixed

 25 August 2008

 - new variable p in output format strings for output offset
 - new format NMEOutputFormatTextCompact with less blank lines
   than NMEOutputFormatText
 - reduced right margin for RTF output
 - tables should be followed by an empty line; fixed
 - --easylink option in NMEMain.c

 15 October 2007

 - unnested unnumbered or numbered list items weren't recognized when
   following sublists; fixed

 12 October 2007

 - NME output better escapes characters when it has to, but usually
   doesn't unless required
 - in NME output, support for mixed nested lists
 - double-slash is considered as a part of URL instead of italic
   markup only if it follows a digit, letter, '+', '-' or '.', and a
   colon; and it precedes a nonblank character; therefore, a tilde
   before the colon, such as abc~://..., restores the meaning of the
   double-slash sequence as italic markup

 9 October 2007

 - in lists, item markup must match, else it is interpreted as plain
   characters or as bold or monospace markup
 - NMEMFC.cpp compiles correctly without _UNICODE

 4 October 2007

 - link encoding for RTF output
 - NMEGtk tuning
 - parHook and divHook bugs, revealed by new option --checkhooks
   of NMEMain.c, fixed

 29 September 2007

 - new high-level function NMEGtkInsert to insert NME text directly
   into a GtkTextBuffer; NMEGtkTest.c modified to use it
 - NMEProcess optionally returns the output length in unicode
   characters, assuming UTF-8 input
 - in NMEMain.c, option --strictcreole doesn't disable escape
   character anymore
 - NMECpp.h uses C++ exceptions for error handling only if
   UseNMECppException is defined; else method getOutput returns an
   error code (the choice is useful for platforms where exceptions
   are not supported or have a large cost)
 - minor bug fixes in NMEStyle.c (plain paragraphs and tables)
 - refined NMEMFC.cpp
 - error codes in hook callbacks weren't handled correctly; fixed
 - support for links and images in span hook and NMEStyle
 - support for links in NMEGtk and NMEMFC
 - in NMECpp and NMEStyleCpp, copy constructors and assignment
   operators
 - NMEEncodeURLFun has an additional argument for
   application-specific data, and structure NMEOutputFormat has an
   additional field for it

 23 August 2007

 - bug in unicode offset computation fixed
 - kNMEProcessOptXRef wasn't handled correctly; fixed
 - blanks following starting = or preceding ending = in titles are
   ignored
 - in NMEGtkApplyStyle, additional argument for the offset of NME
   output in GTK+ text buffer
 - in LaTeX output, braces are now escaped and bold italic is
   suported
 - monospace text wasn't signalled to spanHookFun; fixed
 - MFC glue with NMEMFC.cpp and NMEMFC.h (function
   NMEMFCSetRichText which renders NME text into a CRichEditCtrl)

 5 July 2007

 - option to have verbatim rendered as monospace
   (kNMEProcessOptVerbatimMono)

 4 July 2007

 - callback to check valid wordwrap points
 - bad unicode encoding for RTF fixed
 - tilde is an escape character before any nonblank character,
   including alphanumeric

 19 June 2007

 - support for UCS-16 offset (16-bit unicode offsets when the input
   is encoded in UTF-8)
 - C++ glue for NMEStyle (NMEStyleCpp.h)
 - in block preformatted, when 3 closing braces follow spaces at
   the beginning of a line, one space is removed

 7 June 2007

 - NMEStyle to separate text from style
 - NMEGtkTest.c, a test application for NMEStyle which displays a
   text file with NME markup in a GTK+ 2 window
 - support for placeholders (plugin option to use triple angle
   brackets instead of double angle brackets)
 - bug fixes

 30 May 2007

 - heading of level 1 and 2 following other paragraphs without a
   blank line were always numbered; fixed
 - XHTML-compatible image tag in HTML output
 - plugins source code in separate files
 - in plugin tables, single field for option flags instead of
   separate boolean fields
 - in plugin tables, option flag for plugins between paragraphs
 - characters weren't encoded in preformatted blocks; now there is
   a separate callback
 - new opaque structure NMEContext for output data and other
   contextual information, to reduce the number of arguments passed
   around and to support string output with embedded expressions from
   plugin and autoconvert functions
 - variable o (offset in source code) in embedded expressions,
   useful as unique identifiers for hyperlinks
 - hook functions for paragraphs and sections
 - functions to retrieve current output format and options in
   plugin, autoconvert and hook functions, and to copy input text
   from input to output
 - change in the arguments of NMEProcess, to keep original source
   code
 - table of contents plugin (sample code for the new features)
 - C++ glue (NMECpp.h)

 24 April 2007

 - optional automatic links for URLs beginning with http://,
   https://, ftp://, mailto:
 - optional separate source file for automatic links for camelcase
   and URL
 - options to disable indented paragraphs and definitions lists
 - in NMEMain.c, option --strictnme to disable features not in
   Creole 1.0

 19 April 2007

 - optional numbering of headings, level 1 and/or 2
 - images weren't supported in links; fixed
 - empty cells weren't output correctly in text format; fixed
 - in NMEMain.c, most plugins have reparseOutput set, so that
   word-wrap is applied to output
 - Python glue for the command-line application (nme.py)

 18 April 2007

 - plugins now use the "placeholder" syntax, with double angle
   brackets
 - references to plugin globally-unique identifiers removed
 - option to match only the beginning of plugin names

 16 April 2007

 - doc header and trailer suppressed in plugin output
 - less references to Creole in source code
 - "null" format for producing no output
 - better handling of plugins, especially those which produce NME
   code which must be processed again
 - autoconversion plugins which make substitutions before NME
   conversion (as an example, NMEMain.c has an option to convert
   camelCase words to links)

 30 March 2007

 - wrong encoding of unicode characters >= 32768 for RTF output;
   fixed
 - optional processing of plugin output
 - calendar plugin in NMEMain.c

 23 March 2007

 - support for parenthesis in expressions in output strings
 - replicated strings themselves can contain expressions, and
   syntax for expressions in output strings has been changed to
   support it
 - default font size in structure NMEOutputFormat and support for
   default font size (with nonpositive values, HTML does not contain
   font size specifications at all, relying on the browser defaults)
 - argument withPreAndPostDoc of NMEProcess replaced by options
 - images
 - option to consider line breaks as paragraph separators
 - minor documentation fixes

 20 March 2007

 - Initial release

----------

Copyright Yves Piguet, 4 October 2011. May be distributed with NME or
cited with proper attribution.

