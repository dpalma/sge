# Microsoft Developer Studio Project File - Name="msPlugIn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=msPlugIn - Win32 StaticDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "msPlugIn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "msPlugIn.mak" CFG="msPlugIn - Win32 StaticDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "msPlugIn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msPlugIn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msPlugIn - Win32 Opt" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msPlugIn - Win32 StaticDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msPlugIn - Win32 StaticRelease" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "msPlugIn - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build.vc6\Release"
# PROP Intermediate_Dir "..\Build.vc6\Release\msPlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\3rdparty\ms3dsdk\msLib" /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /D "NDEBUG" /D "_USRDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "_WINDLL" /D "_AFXDLL" /Yu"StdAfx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 msModelLib.lib $(OutDir)/tinyxml.lib opengl32.lib glu32.lib winmm.lib /nologo /dll /map:"..\Build.vc6\Release/msSGEExporter.map" /machine:I386 /out:"..\Build.vc6\Release/msSGEExporter.dll" /libpath:"..\3rdparty\ms3dsdk\msLib\lib" /opt:ref
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "msPlugIn - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\Debug"
# PROP Intermediate_Dir "..\Build.vc6\Debug\msPlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I "..\3rdparty\ms3dsdk\msLib" /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /D "_DEBUG" /D "_USRDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "_WINDLL" /D "_AFXDLL" /Yu"StdAfx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msModelLib.lib $(OutDir)/tinyxml.lib opengl32.lib glu32.lib winmm.lib /nologo /dll /map:"..\Build.vc6\Debug/msSGEExporter.map" /debug /machine:I386 /out:"..\Build.vc6\Debug/msSGEExporter.dll" /pdbtype:sept /libpath:"..\3rdparty\ms3dsdk\msLib\lib"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "msPlugIn - Win32 Opt"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Opt"
# PROP BASE Intermediate_Dir "Opt"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\Opt"
# PROP Intermediate_Dir "..\Build.vc6\Opt\msPlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /O2 /I "..\3rdparty\ms3dsdk\msLib" /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /D "_USRDLL" /D "STRICT" /D "_WINDLL" /D "_AFXDLL" /D "_WINDOWS" /D "NDEBUG" /D "WIN32" /D "_MBCS" /Yu"StdAfx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\3rdparty\dx7sdk\lib"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 msModelLib.lib $(OutDir)/tinyxml.lib opengl32.lib glu32.lib winmm.lib /nologo /dll /profile /map:"..\Build.vc6\Opt/msSGEExporter.map" /debug /machine:I386 /out:"..\Build.vc6\Opt/msSGEExporter.dll" /libpath:"..\3rdparty\ms3dsdk\msLib\lib"

!ELSEIF  "$(CFG)" == "msPlugIn - Win32 StaticDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "msPlugIn___Win32_StaticDebug"
# PROP BASE Intermediate_Dir "msPlugIn___Win32_StaticDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\StaticDebug"
# PROP Intermediate_Dir "..\Build.vc6\StaticDebug\msPlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I "..\3rdparty\ms3dsdk\msLib" /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /D "_DEBUG" /D "STATIC_BUILD" /D "_USRDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "_WINDLL" /Yu"StdAfx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 $(OutDir)\cppunit.lib msModelLib.lib $(OutDir)/tinyxml.lib opengl32.lib glu32.lib winmm.lib /nologo /dll /map:"..\Build.vc6\StaticDebug/msSGEExporter.map" /debug /machine:I386 /out:"..\Build.vc6\StaticDebug/msSGEExporter.dll" /libpath:"..\3rdparty\ms3dsdk\msLib\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "msPlugIn - Win32 StaticRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "msPlugIn___Win32_StaticRelease"
# PROP BASE Intermediate_Dir "msPlugIn___Win32_StaticRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build.vc6\StaticRelease"
# PROP Intermediate_Dir "..\Build.vc6\StaticRelease\msPlugIn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdhdr.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\3rdparty\ms3dsdk\msLib" /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /D "NDEBUG" /D "STATIC_BUILD" /D "_USRDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "_WINDLL" /Yu"StdAfx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 msModelLib.lib $(OutDir)/tinyxml.lib opengl32.lib glu32.lib winmm.lib /nologo /dll /map:"..\Build.vc6\StaticRelease/msSGEExporter.map" /machine:I386 /out:"..\Build.vc6\StaticRelease/msSGEExporter.dll" /libpath:"..\3rdparty\ms3dsdk\msLib\lib" /opt:ref
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "msPlugIn - Win32 Release"
# Name "msPlugIn - Win32 Debug"
# Name "msPlugIn - Win32 Opt"
# Name "msPlugIn - Win32 StaticDebug"
# Name "msPlugIn - Win32 StaticRelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ModelTreeInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\msPlugIn.def
# End Source File
# Begin Source File

SOURCE=.\msPlugIn.rc
# End Source File
# Begin Source File

SOURCE=.\msPlugInApp.cpp
# End Source File
# Begin Source File

SOURCE=.\msPlugInExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\msPlugInImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"StdAfx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ModelTreeInfo.h
# End Source File
# Begin Source File

SOURCE=.\msPlugInApp.h
# End Source File
# Begin Source File

SOURCE=.\msPlugInExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\msPlugInImpl.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\addanim.bmp
# End Source File
# Begin Source File

SOURCE=.\res\addanimd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\addanimf.bmp
# End Source File
# Begin Source File

SOURCE=.\res\addanimu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\addanimx.bmp
# End Source File
# Begin Source File

SOURCE=.\res\msPlugIn.rc2
# End Source File
# Begin Source File

SOURCE=.\res\rmanim.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rmanimd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rmanimf.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rmanimu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rmanimx.bmp
# End Source File
# End Group
# End Target
# End Project
