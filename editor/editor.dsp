# Microsoft Developer Studio Project File - Name="editor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=editor - Win32 StaticDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "editor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "editor.mak" CFG="editor - Win32 StaticDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "editor - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "editor - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "editor - Win32 Opt" (based on "Win32 (x86) Application")
!MESSAGE "editor - Win32 StaticDebug" (based on "Win32 (x86) Application")
!MESSAGE "editor - Win32 StaticRelease" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "editor - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build.vc6\Release"
# PROP Intermediate_Dir "..\Build.vc6\Release\editor"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdhdr.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /I "..\3rdparty\tinyxml" /D "NDEBUG" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib winmm.lib $(OutDir)\lua.lib $(OutDir)\tinyxml.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "editor - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\Debug"
# PROP Intermediate_Dir "..\Build.vc6\Debug\editor"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /I "..\3rdparty\tinyxml" /D "_DEBUG" /D "_AFXDLL" /D "HAVE_CPPUNIT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(OutDir)\cppunit.lib $(OutDir)\testrunner.lib opengl32.lib glu32.lib winmm.lib $(OutDir)\lua.lib $(OutDir)\tinyxml.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "editor - Win32 Opt"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "editor___Win32_Opt"
# PROP BASE Intermediate_Dir "editor___Win32_Opt"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build.vc6\Opt"
# PROP Intermediate_Dir "..\Build.vc6\Opt\editor"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdhdr.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /I "..\3rdparty\tinyxml" /D "NDEBUG" /D "_AFXDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 opengl32.lib glu32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib winmm.lib $(OutDir)\lua.lib $(OutDir)\tinyxml.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "editor - Win32 StaticDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "editor___Win32_StaticDebug"
# PROP BASE Intermediate_Dir "editor___Win32_StaticDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\StaticDebug"
# PROP Intermediate_Dir "..\Build.vc6\StaticDebug\editor"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /I "..\3rdparty\tinyxml" /D "STATIC_BUILD" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 opengl32.lib glu32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(OutDir)\cppunit.lib opengl32.lib glu32.lib winmm.lib $(OutDir)\lua.lib $(OutDir)\tinyxml.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "editor - Win32 StaticRelease"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "editor___Win32_StaticRelease"
# PROP BASE Intermediate_Dir "editor___Win32_StaticRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build.vc6\StaticRelease"
# PROP Intermediate_Dir "..\Build.vc6\StaticRelease\editor"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdhdr.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\tech\include" /I "..\engine\include" /I "..\render\include" /I "..\3rdparty\tinyxml" /D "STATIC_BUILD" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 opengl32.lib glu32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib winmm.lib $(OutDir)\lua.lib $(OutDir)\tinyxml.lib /nologo /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "editor - Win32 Release"
# Name "editor - Win32 Debug"
# Name "editor - Win32 Opt"
# Name "editor - Win32 StaticDebug"
# Name "editor - Win32 StaticRelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\cmds.cpp
# End Source File
# Begin Source File

SOURCE=.\editor.rc
# End Source File
# Begin Source File

SOURCE=.\editorApp.cpp
# End Source File
# Begin Source File

SOURCE=.\editorCtrlBars.cpp
# End Source File
# Begin Source File

SOURCE=.\editorDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\editorTile.cpp
# End Source File
# Begin Source File

SOURCE=.\editorTileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\editorTileSet.cpp
# End Source File
# Begin Source File

SOURCE=.\editorView.cpp
# End Source File
# Begin Source File

SOURCE=.\glView.cpp
# End Source File
# Begin Source File

SOURCE=.\heightmap.cpp
# End Source File
# Begin Source File

SOURCE=.\historywnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MapSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputBar.cpp
# End Source File
# Begin Source File

SOURCE=.\scbarcf.cpp
# ADD CPP /Yu"stdhdr.h"
# End Source File
# Begin Source File

SOURCE=.\scbarg.cpp
# ADD CPP /Yu"stdhdr.h"
# End Source File
# Begin Source File

SOURCE=.\scrollwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\sizecbar.cpp
# ADD CPP /Yu"stdhdr.h"
# End Source File
# Begin Source File

SOURCE=.\splashwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\stdhdr.cpp
# ADD CPP /Yc"stdhdr.h"
# End Source File
# Begin Source File

SOURCE=.\tiledground.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolPaletteBar.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\aboutdlg.h
# End Source File
# Begin Source File

SOURCE=.\afxcomtools.h
# End Source File
# Begin Source File

SOURCE=.\BitmapUtils.h
# End Source File
# Begin Source File

SOURCE=.\editorapi.h
# End Source File
# Begin Source File

SOURCE=.\editorApp.h
# End Source File
# Begin Source File

SOURCE=.\editorCtrlBars.h
# End Source File
# Begin Source File

SOURCE=.\editorDoc.h
# End Source File
# Begin Source File

SOURCE=.\editorguids.h
# End Source File
# Begin Source File

SOURCE=.\editorTile.h
# End Source File
# Begin Source File

SOURCE=.\editorTileManager.h
# End Source File
# Begin Source File

SOURCE=.\editorTileSet.h
# End Source File
# Begin Source File

SOURCE=.\editorView.h
# End Source File
# Begin Source File

SOURCE=.\glView.h
# End Source File
# Begin Source File

SOURCE=.\heightmap.h
# End Source File
# Begin Source File

SOURCE=.\historywnd.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MapSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\OutputBar.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\scbarcf.h
# End Source File
# Begin Source File

SOURCE=.\scbarg.h
# End Source File
# Begin Source File

SOURCE=.\scrollwnd.h
# End Source File
# Begin Source File

SOURCE=.\sizecbar.h
# End Source File
# Begin Source File

SOURCE=.\splashwnd.h
# End Source File
# Begin Source File

SOURCE=.\stdhdr.h
# End Source File
# Begin Source File

SOURCE=.\tiledground.h
# End Source File
# Begin Source File

SOURCE=.\ToolPaletteBar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\editor.ico
# End Source File
# Begin Source File

SOURCE=.\res\editor.rc2
# End Source File
# Begin Source File

SOURCE=.\res\editorDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# End Group
# End Target
# End Project
