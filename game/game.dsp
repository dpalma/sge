# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=game - Win32 StaticDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 StaticDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "game - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "game - Win32 StaticRelease" (based on "Win32 (x86) Application")
!MESSAGE "game - Win32 StaticDebug" (based on "Win32 (x86) Application")
!MESSAGE "game - Win32 Opt" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build\Release"
# PROP Intermediate_Dir "..\Build\Release\game"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\3rdparty\lua\include" /I "..\engine\include" /I "..\render\include" /I "..\tech\include" /I "..\3rdparty\tinyxml" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib opengl32.lib winmm.lib $(OutDir)\tinyxml.lib $(OutDir)\lua.lib /nologo /subsystem:windows /map:"..\Build\Release/game.map" /machine:I386

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\Debug"
# PROP Intermediate_Dir "..\Build\Debug\game"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I "..\3rdparty\lua\include" /I "..\engine\include" /I "..\render\include" /I "..\tech\include" /I "..\3rdparty\tinyxml" /D "_DEBUG" /D "HAVE_CPPUNIT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(OutDir)\cppunit.lib kernel32.lib user32.lib gdi32.lib opengl32.lib winmm.lib $(OutDir)\tinyxml.lib $(OutDir)\lua.lib /nologo /subsystem:windows /map:"..\Build\Debug/game.map" /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "game - Win32 StaticRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_StaticRelease"
# PROP BASE Intermediate_Dir "game___Win32_StaticRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build\StaticRelease"
# PROP Intermediate_Dir "..\Build\StaticRelease\game"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\lua\include" /I "..\tech\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\3rdparty\lua\include" /I "..\engine\include" /I "..\render\include" /I "..\tech\include" /I "..\3rdparty\tinyxml" /D "NDEBUG" /D "STATIC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib $(OutDir)\tinyxml.lib $(OutDir)\lua.lib /nologo /subsystem:windows /map:"..\Build\StaticRelease/game.map" /machine:I386

!ELSEIF  "$(CFG)" == "game - Win32 StaticDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "game___Win32_StaticDebug"
# PROP BASE Intermediate_Dir "game___Win32_StaticDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\StaticDebug"
# PROP Intermediate_Dir "..\Build\StaticDebug\game"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\lua\include" /I "..\tech\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I "..\3rdparty\lua\include" /I "..\engine\include" /I "..\render\include" /I "..\tech\include" /I "..\3rdparty\tinyxml" /D "STATIC_BUILD" /D "_DEBUG" /D "HAVE_CPPUNIT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /Yu"stdhdr.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(OutDir)\cppunit.lib kernel32.lib user32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib $(OutDir)\tinyxml.lib $(OutDir)\lua.lib /nologo /subsystem:windows /map:"..\Build\StaticDebug/game.map" /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "game - Win32 Opt"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_Opt"
# PROP BASE Intermediate_Dir "game___Win32_Opt"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build\Opt"
# PROP Intermediate_Dir "..\Build\Opt\game"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\lua\include" /I "..\tech\include" /I "..\3rdparty\tinyxml" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "TIXML_USE_STL" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /O2 /I "..\3rdparty\lua\include" /I "..\engine\include" /I "..\render\include" /I "..\tech\include" /I "..\3rdparty\tinyxml" /D "TIXML_USE_STL" /D "_WINDOWS" /D "NDEBUG" /D "WIN32" /D "_MBCS" /Yu"stdhdr.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib $(OutDir)\tinyxml.lib /nologo /subsystem:windows /map /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib opengl32.lib winmm.lib $(OutDir)\tinyxml.lib $(OutDir)\lua.lib /nologo /subsystem:windows /profile /map:"..\Build\Opt/game.map" /debug /machine:I386

!ENDIF 

# Begin Target

# Name "game - Win32 Release"
# Name "game - Win32 Debug"
# Name "game - Win32 StaticRelease"
# Name "game - Win32 StaticDebug"
# Name "game - Win32 Opt"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\boundingvolume.cpp
# End Source File
# Begin Source File

SOURCE=.\cameracontroller.cpp
# End Source File
# Begin Source File

SOURCE=.\frustum.cpp
# End Source File
# Begin Source File

SOURCE=.\game.rc
# End Source File
# Begin Source File

SOURCE=.\groundtiled.cpp
# End Source File
# Begin Source File

SOURCE=.\heightmap.cpp
# End Source File
# Begin Source File

SOURCE=.\input.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\raycast.cpp
# End Source File
# Begin Source File

SOURCE=.\scenecamera.cpp
# End Source File
# Begin Source File

SOURCE=.\scenemesh.cpp
# End Source File
# Begin Source File

SOURCE=.\scenenode.cpp
# End Source File
# Begin Source File

SOURCE=.\script.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptvm.cpp
# End Source File
# Begin Source File

SOURCE=.\stdhdr.cpp
# ADD CPP /Yc"stdhdr.h"
# End Source File
# Begin Source File

SOURCE=.\syslinux.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\syswin.cpp
# End Source File
# Begin Source File

SOURCE=.\ui.cpp
# End Source File
# Begin Source File

SOURCE=.\uievent.cpp
# End Source File
# Begin Source File

SOURCE=.\uimgr.cpp
# End Source File
# Begin Source File

SOURCE=.\uiparse.cpp
# End Source File
# Begin Source File

SOURCE=.\uirender.cpp
# End Source File
# Begin Source File

SOURCE=.\uiwidgets.cpp
# End Source File
# Begin Source File

SOURCE=.\uiwidgutils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\boundingvolume.h
# End Source File
# Begin Source File

SOURCE=.\cameracontroller.h
# End Source File
# Begin Source File

SOURCE=.\frustum.h
# End Source File
# Begin Source File

SOURCE=.\gameguids.h
# End Source File
# Begin Source File

SOURCE=.\gcommon.h
# End Source File
# Begin Source File

SOURCE=.\ggl.h
# End Source File
# Begin Source File

SOURCE=.\groundtiled.h
# End Source File
# Begin Source File

SOURCE=.\heightmap.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\raycast.h
# End Source File
# Begin Source File

SOURCE=.\scenecamera.h
# End Source File
# Begin Source File

SOURCE=.\scenemesh.h
# End Source File
# Begin Source File

SOURCE=.\scenenode.h
# End Source File
# Begin Source File

SOURCE=.\script.h
# End Source File
# Begin Source File

SOURCE=.\scriptvm.h
# End Source File
# Begin Source File

SOURCE=.\stdhdr.h
# End Source File
# Begin Source File

SOURCE=.\sys.h
# End Source File
# Begin Source File

SOURCE=.\ui.h
# End Source File
# Begin Source File

SOURCE=.\uievent.h
# End Source File
# Begin Source File

SOURCE=.\uimgr.h
# End Source File
# Begin Source File

SOURCE=.\uiparse.h
# End Source File
# Begin Source File

SOURCE=.\uirender.h
# End Source File
# Begin Source File

SOURCE=.\uitypes.h
# End Source File
# Begin Source File

SOURCE=.\uiwidgets.h
# End Source File
# Begin Source File

SOURCE=.\uiwidgutils.h
# End Source File
# Begin Source File

SOURCE=.\uiwidgutilstem.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\game.ico
# End Source File
# End Group
# Begin Group "Lua Scripts"

# PROP Default_Filter "lua"
# Begin Source File

SOURCE=.\autoexec.lua

!IF  "$(CFG)" == "game - Win32 Release"

# Begin Custom Build - Compiling $(InputPath)
OutDir=.\..\Build\Release
InputPath=.\autoexec.lua
InputName=autoexec

"$(OutDir)\$(InputName).luo" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\luac.exe -o $(OutDir)\$(InputName).luo $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# Begin Custom Build - Compiling $(InputPath)
OutDir=.\..\Build\Debug
InputPath=.\autoexec.lua
InputName=autoexec

"$(OutDir)\$(InputName).luo" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\luac.exe -o $(OutDir)\$(InputName).luo $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "game - Win32 StaticRelease"

# Begin Custom Build - Compiling $(InputPath)
OutDir=.\..\Build\StaticRelease
InputPath=.\autoexec.lua
InputName=autoexec

"$(OutDir)\$(InputName).luo" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\luac.exe -o $(OutDir)\$(InputName).luo $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "game - Win32 StaticDebug"

# Begin Custom Build - Compiling $(InputPath)
OutDir=.\..\Build\StaticDebug
InputPath=.\autoexec.lua
InputName=autoexec

"$(OutDir)\$(InputName).luo" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\luac.exe -o $(OutDir)\$(InputName).luo $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "game - Win32 Opt"

# Begin Custom Build - Compiling $(InputPath)
OutDir=.\..\Build\Opt
InputPath=.\autoexec.lua
InputName=autoexec

"$(OutDir)\$(InputName).luo" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\luac.exe -o $(OutDir)\$(InputName).luo $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
