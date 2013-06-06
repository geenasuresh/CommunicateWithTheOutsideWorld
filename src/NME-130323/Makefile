# makefile for NME command-line application and NME distribution
# Copyright 2007-2012, Yves Piguet

# to create distribution as NME-xxx: "make distrib DISTRIB=NME-xxx"

TARGET ?= $(shell uname)
ifeq ($(TARGET), Darwin)
PKGCONFIG = /sw/bin/pkg-config
else
PKGCONFIG = pkg-config
endif

ZLIB ?= zlib

CC = gcc

VPATH = Src:$(ZLIB):$(ZLIB)/contrib/minizip
CINCL = -ISrc -I$(ZLIB) -I$(ZLIB)/contrib/minizip
CFLAGS = -g $(CINCL) -DUSE_FILE32API
CPPFLAGS = -g $(CINCL)

ifeq ($(TARGET), Darwin)
# for minizip
CFLAGS += -DUSE_FILE32API
endif

DISTRIB ?= NME-distrib

objects = NME.o NMEAutolink.o \
	NMEPluginCalendar.o NMEPluginRaw.o NMEPluginReverse.o NMEPluginRot13.o \
	NMEPluginUppercase.o NMEPluginTOC.o NMEPluginWiki.o \
	NMEEPub.o NMETest.o

zipObjects = adler32.o crc32.o deflate.o ioapi.o trees.o zip.o zutil.o

docnme = readme.nme markup.nme
docprocessed = $(docnme:.nme=.txt) $(docnme:.nme=.html)
doc = $(docnme) $(docprocessed)

nme: $(objects) NMEMain.o
	$(CC) $(LDFLAGS) -o $@ $^

nmecpp: NME.o NMEStyle.o NMECppTest.o
	$(CXX) $(LDFLAGS) -o $@ $^

nmegtk: NMEGtkTest.o NME.o NMEStyle.o NMEGtk.o
	$(CC) -o $@ $^ `$(PKGCONFIG) --libs gtk+-2.0`

nmeepub: NMEEPubMain.o NME.o NMEEPub.o NMEAutolink.o NE.o $(zipObjects)
	$(CC) -o $@ $^

nmerandom: NMERandomGen.o
	$(CC) -o $@ $^

NMEGtkTest.o: NMEGtkTest.c
	$(CC) -c $(CFLAGS) `$(PKGCONFIG) --cflags gtk+-2.0` $^

NMEGtk.o: NMEGtk.c
	$(CC) -c $(CFLAGS) `$(PKGCONFIG) --cflags gtk+-2.0` $^

#-lgtk-x11-2.0 -lgobject-2.0.0 -lglib-2.0 -lpango-1.0

# header dependencies
NME.o: NME.h
NMEStyle.o: NME.h NMEStyle.h
NMEAutolink.o: NME.h NMEAutolink.h
NMEPluginCalendar.o: NME.h NMEPluginCalendar.h
NMEPluginRaw.o: NME.h NMEPluginRaw.h
NMEPluginReverse.o: NME.h NMEPluginReverse.h
NMEPluginRot13.o: NME.h NMEPluginRot13.h
NMEPluginUppercase.o: NME.h NMEPluginUppercase.h
NMEPluginTOC.o: NME.h NMEPluginTOC.h
NMEPluginWiki.o: NME.h NMEPluginWiki.h
NMEMain.o: NME.h NMEAutolink.h NMEEPub.h \
	NMEPluginCalendar.h NMEPluginRaw.h \
	NMEPluginReverse.h NMEPluginRot13.h NMEPluginUppercase.h \
	NMEPluginTOC.h NMEPluginWiki.h \
	NMETest.h
NMEEPubMain.o: NME.h NMEAutolink.h NE.h NMEEPub.h
NMEEPub.o: NMEEPub.h
NE.o: NE.h

.PHONY: distrib
distrib: NME.c NME.h NMEAutolink.c NMEAutolink.h NMEMain.c NMEEPubMain.c NMEEPub.c \
		NMEGtk.c NMEGtk.h NMEMFC.cpp NMEMFC.h \
		NMEPluginReverse.c NMEPluginRot13.c NMEPluginUppercase.c \
		NMEPluginCalendar.c NMEPluginRaw.c \
		NMEPluginReverse.h NMEPluginRot13.h NMEPluginUppercase.h \
		NMEPluginCalendar.h NMEPluginRaw.h \
		NE.c NE.h \
		$(doc)
	rm -Rf $(DISTRIB)
	mkdir $(DISTRIB)
	mkdir $(DISTRIB)/Src
	cp Makefile $(docprocessed) $(DISTRIB)
	cp readme.nme markup.nme $(DISTRIB)
	cp Src/NME.[ch] Src/NMEStyle.[ch] \
			Src/NMEAutolink.[ch] Src/NMEPluginReverse.[ch] \
			Src/NMEPluginRot13.[ch] Src/NMEPluginUppercase.[ch] \
			Src/NMEPluginCalendar.[ch] Src/NMEPluginRaw.[ch] \
			Src/NMEPluginTOC.[ch] Src/NMEPluginWiki.[ch] \
			Src/NMETest.[ch] \
			Src/NMECpp.h Src/NMEStyleCpp.h \
			Src/NMEGtk.[ch] Src/NMEMFC.cpp Src/NMEMFC.h Src/NMEObjC.[mh] \
			Src/NMECppTest.cpp Src/NMEErrorCpp.h \
			Src/NMEMain.c Src/NMEGtkTest.c Src/NMERandomGen.c \
			Src/NMEEPubMain.c Src/NMEEPub.[ch] Src/NE.[ch] \
			$(DISTRIB)/Src
	mkdir $(DISTRIB)/BuildWin
	cp BuildWin/BuildAll.bat BuildWin/NME.sln BuildWin/ReadMe.txt \
			BuildWin/NMEEPubTool.vcxproj BuildWin/NMETool.vcxproj \
			$(DISTRIB)/BuildWin
	sed -n -e '/Redistributions of/,/SUCH DAMAGE/p' Src/NME.h >$(DISTRIB)/LICENSE.txt
	rm -f $(DISTRIB).zip
	zip -r $(DISTRIB).zip $(DISTRIB)

%.d: %.c
%.d: %.cpp
	set -e; $(CC) -MM $(CFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

sinclude $(objects:.o=.d) NMEMain.d NMETest.d

%.txt: %.nme nme
	./nme --text <$< >$@ || (rm -f $@; false)

%.html: %.nme nme
	./nme --html <$< >$@ || (rm -f $@; false)

.PHONY: clean
clean:
	rm -f $(objects) $(docprocessed)
