Projects for Visual Studio 2010

To compile nme.exe and nmeepub.exe:

- download the most recent version of zlib from http://www.zlib.org (currently zlib 1.2.5);

- expand it;

- rename the directory "zlib" (without version number) and move it at the same level as nme:

(root)
  nme
    Src
    Win32
    ...
  zlib
    contrib
      minizip
      ...
    ...

- open NME.sln in Visual Studio 2010 and Rebuild Solution

OR

- menu Start > Microsoft Visual Studio 2010 > Visual Studio Tools > Visual Studio Command Prompt; cd to NME\BuildWin, and type BuildAll.bat

zlib is needed only for NMEEPubTool.vcxproj; you don't have to install it if you want to compile only NMETool.vcxproj.

YP, 3 Oct 2011
