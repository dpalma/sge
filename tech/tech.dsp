# Microsoft Developer Studio Project File - Name="tech" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=tech - Win32 StaticDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tech.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tech.mak" CFG="tech - Win32 StaticDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tech - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tech - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tech - Win32 Opt" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tech - Win32 StaticDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "tech - Win32 StaticRelease" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "tech - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build\Release"
# PROP Intermediate_Dir "..\Build\Release\tech"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TECH_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\include" /I "..\3rdparty\zlib\include" /D "NDEBUG" /D "_USRDLL" /D "TECH_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /Yu"stdhdr.h" /FD /c
# SUBTRACT CPP /Fr
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 $(OutDir)\zlibwapi.lib kernel32.lib user32.lib gdi32.lib winmm.lib opengl32.lib /nologo /dll /map:"..\Build\Release/tech.map" /machine:I386 /opt:ref
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tech - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\Debug"
# PROP Intermediate_Dir "..\Build\Debug\tech"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TECH_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I ".\include" /I "..\3rdparty\zlib\include" /D "_DEBUG" /D "_USRDLL" /D "TECH_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "HAVE_CPPUNIT" /Yu"stdhdr.h" /FD /GZ /c
# SUBTRACT CPP /Fr
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(OutDir)\cppunit.lib $(OutDir)\zlibwapi.lib kernel32.lib user32.lib gdi32.lib winmm.lib opengl32.lib /nologo /dll /map:"..\Build\Debug/tech.map" /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "tech - Win32 Opt"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Opt"
# PROP BASE Intermediate_Dir "Opt"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\Opt"
# PROP Intermediate_Dir "..\Build\Opt\tech"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TECH_EXPORTS" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /O2 /I ".\include" /I "..\3rdparty\zlib\include" /D "_USRDLL" /D "TECH_EXPORTS" /D "STRICT" /D "_WINDOWS" /D "NDEBUG" /D "WIN32" /D "_MBCS" /Yu"stdhdr.h" /FD /c
# SUBTRACT CPP /Fr
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\3rdparty\dx7sdk\lib"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 $(OutDir)\zlibwapi.lib kernel32.lib user32.lib gdi32.lib winmm.lib opengl32.lib /nologo /dll /profile /map:"..\Build\Opt/tech.map" /debug /machine:I386

!ELSEIF  "$(CFG)" == "tech - Win32 StaticDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Tech___Win32_StaticDebug"
# PROP BASE Intermediate_Dir "Tech___Win32_StaticDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\StaticDebug"
# PROP Intermediate_Dir "..\Build\StaticDebug\tech"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
LINK32=link.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TECH_EXPORTS" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I ".\include" /I "..\3rdparty\zlib\include" /D "STATIC_BUILD" /D "_DEBUG" /D "_USRDLL" /D "TECH_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "HAVE_CPPUNIT" /Yu"stdhdr.h" /FD /GZ /c
# SUBTRACT CPP /Fr
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "tech - Win32 StaticRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Tech___Win32_StaticRelease"
# PROP BASE Intermediate_Dir "Tech___Win32_StaticRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build\StaticRelease"
# PROP Intermediate_Dir "..\Build\StaticRelease\tech"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
LINK32=link.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TECH_EXPORTS" /Yu"stdhdr.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /I "..\3rdparty\zlib\include" /D "NDEBUG" /D "STATIC_BUILD" /D "_USRDLL" /D "TECH_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /Yu"stdhdr.h" /FD /c
# SUBTRACT CPP /Fr
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "tech - Win32 Release"
# Name "tech - Win32 Debug"
# Name "tech - Win32 Opt"
# Name "tech - Win32 StaticDebug"
# Name "tech - Win32 StaticRelease"
# Begin Group ".\src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\comtools.cpp
# End Source File
# Begin Source File

SOURCE=.\src\config.cpp
# End Source File
# Begin Source File

SOURCE=.\src\configstore.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fileiter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\filepath.cpp
# End Source File
# Begin Source File

SOURCE=.\src\filereadwrite.cpp
# End Source File
# Begin Source File

SOURCE=.\src\filespec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\globalobjreg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\graphtest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\hash.cpp
# End Source File
# Begin Source File

SOURCE=.\src\hashtbltest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\matrix4.cpp
# End Source File
# Begin Source File

SOURCE=.\src\memmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\parse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\prime.cpp
# End Source File
# Begin Source File

SOURCE=.\src\quat.cpp
# End Source File
# Begin Source File

SOURCE=.\src\rand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\resmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\scriptvar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stdhdr.cpp
# ADD CPP /Yc"stdhdr.h"
# End Source File
# Begin Source File

SOURCE=.\src\techdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\techtime.cpp
# End Source File
# Begin Source File

SOURCE=.\src\windoww32.cpp
# End Source File
# Begin Source File

SOURCE=.\src\windowx11.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\config.h
# End Source File
# Begin Source File

SOURCE=.\src\configstore.h
# End Source File
# Begin Source File

SOURCE=.\src\filereadwrite.h
# End Source File
# Begin Source File

SOURCE=.\src\hashtbl.h
# End Source File
# Begin Source File

SOURCE=.\src\hashtbltem.h
# End Source File
# Begin Source File

SOURCE=.\src\stdhdr.h
# End Source File
# End Group
# End Group
# Begin Group ".\include"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\combase.h
# End Source File
# Begin Source File

SOURCE=.\include\comtools.h
# End Source File
# Begin Source File

SOURCE=.\include\configapi.h
# End Source File
# Begin Source File

SOURCE=.\include\connpt.h
# End Source File
# Begin Source File

SOURCE=.\include\connptimpl.h
# End Source File
# Begin Source File

SOURCE=.\include\constraints.h
# End Source File
# Begin Source File

SOURCE=.\include\dbgalloc.h
# End Source File
# Begin Source File

SOURCE=.\include\digraph.h
# End Source File
# Begin Source File

SOURCE=.\include\fileconst.h
# End Source File
# Begin Source File

SOURCE=.\include\fileiter.h
# End Source File
# Begin Source File

SOURCE=.\include\filepath.h
# End Source File
# Begin Source File

SOURCE=.\include\filespec.h
# End Source File
# Begin Source File

SOURCE=.\include\globalobj.h
# End Source File
# Begin Source File

SOURCE=.\include\globalobjreg.h
# End Source File
# Begin Source File

SOURCE=.\include\hash.h
# End Source File
# Begin Source File

SOURCE=.\include\keys.h
# End Source File
# Begin Source File

SOURCE=.\include\matrix4.h
# End Source File
# Begin Source File

SOURCE=.\include\memmanager.h
# End Source File
# Begin Source File

SOURCE=.\include\parse.h
# End Source File
# Begin Source File

SOURCE=.\include\pixelformat.h
# End Source File
# Begin Source File

SOURCE=.\include\quat.h
# End Source File
# Begin Source File

SOURCE=.\include\readwriteapi.h
# End Source File
# Begin Source File

SOURCE=.\include\resmgr.h
# End Source File
# Begin Source File

SOURCE=.\include\scriptapi.h
# End Source File
# Begin Source File

SOURCE=.\include\scriptvar.h
# End Source File
# Begin Source File

SOURCE=.\include\str.h
# End Source File
# Begin Source File

SOURCE=.\include\techdebug.h
# End Source File
# Begin Source File

SOURCE=.\include\techdll.h
# End Source File
# Begin Source File

SOURCE=.\include\techguids.h
# End Source File
# Begin Source File

SOURCE=.\include\techmath.h
# End Source File
# Begin Source File

SOURCE=.\include\techtime.h
# End Source File
# Begin Source File

SOURCE=.\include\techtypes.h
# End Source File
# Begin Source File

SOURCE=.\include\toposort.h
# End Source File
# Begin Source File

SOURCE=.\include\undbgalloc.h
# End Source File
# Begin Source File

SOURCE=.\include\vec2.h
# End Source File
# Begin Source File

SOURCE=.\include\vec3.h
# End Source File
# Begin Source File

SOURCE=.\include\vec4.h
# End Source File
# Begin Source File

SOURCE=.\include\window.h
# End Source File
# End Group
# End Target
# End Project
