# Microsoft Developer Studio Project File - Name="engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=engine - Win32 StaticDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak" CFG="engine - Win32 StaticDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "engine - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "engine - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "engine - Win32 Opt" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "engine - Win32 StaticDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "engine - Win32 StaticRelease" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build\Release"
# PROP Intermediate_Dir "..\Build\Release\engine"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ENGINE_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\include" /I "..\tech\include" /I "..\render\include" /D "NDEBUG" /D "_USRDLL" /D "ENGINE_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /Yu"stdhdr.h" /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib opengl32.lib /nologo /dll /map:"..\Build\Release/engine.map" /machine:I386 /opt:ref
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\Debug"
# PROP Intermediate_Dir "..\Build\Debug\engine"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ENGINE_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I ".\include" /I "..\tech\include" /I "..\render\include" /D "_DEBUG" /D "_USRDLL" /D "ENGINE_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "HAVE_CPPUNIT" /Yu"stdhdr.h" /FD /GZ /c
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
# ADD LINK32 $(OutDir)\cppunit.lib kernel32.lib user32.lib gdi32.lib winmm.lib opengl32.lib /nologo /dll /map:"..\Build\Debug/engine.map" /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "engine - Win32 Opt"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Opt"
# PROP BASE Intermediate_Dir "Opt"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\Opt"
# PROP Intermediate_Dir "..\Build\Opt\engine"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ENGINE_EXPORTS" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /O2 /I ".\include" /I "..\tech\include" /I "..\render\include" /D "_USRDLL" /D "ENGINE_EXPORTS" /D "STRICT" /D "_WINDOWS" /D "NDEBUG" /D "WIN32" /D "_MBCS" /Yu"stdhdr.h" /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib opengl32.lib /nologo /dll /profile /map:"..\Build\Opt/engine.map" /debug /machine:I386

!ELSEIF  "$(CFG)" == "engine - Win32 StaticDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Tech___Win32_StaticDebug"
# PROP BASE Intermediate_Dir "Tech___Win32_StaticDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build\StaticDebug"
# PROP Intermediate_Dir "..\Build\StaticDebug\engine"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
LINK32=link.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ENGINE_EXPORTS" /Yu"stdhdr.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\3rdparty\cppunit\include" /I ".\include" /I "..\tech\include" /I "..\render\include" /D "STATIC_BUILD" /D "_DEBUG" /D "_USRDLL" /D "ENGINE_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "HAVE_CPPUNIT" /Yu"stdhdr.h" /FD /GZ /c
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

!ELSEIF  "$(CFG)" == "engine - Win32 StaticRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Tech___Win32_StaticRelease"
# PROP BASE Intermediate_Dir "Tech___Win32_StaticRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build\StaticRelease"
# PROP Intermediate_Dir "..\Build\StaticRelease\engine"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
LINK32=link.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ENGINE_EXPORTS" /Yu"stdhdr.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /I "..\tech\include" /I "..\render\include" /D "NDEBUG" /D "STATIC_BUILD" /D "_USRDLL" /D "ENGINE_EXPORTS" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /Yu"stdhdr.h" /FD /c
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

# Name "engine - Win32 Release"
# Name "engine - Win32 Debug"
# Name "engine - Win32 Opt"
# Name "engine - Win32 StaticDebug"
# Name "engine - Win32 StaticRelease"
# Begin Group ".\src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\3ds.cpp
# End Source File
# Begin Source File

SOURCE=.\src\animation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\frustum.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ms3d.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ms3dread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ray.cpp
# End Source File
# Begin Source File

SOURCE=.\src\scene.cpp
# End Source File
# Begin Source File

SOURCE=.\src\scenecamera.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sceneentity.cpp
# End Source File
# Begin Source File

SOURCE=.\src\scenelayer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sim.cpp
# End Source File
# Begin Source File

SOURCE=.\src\skeleton.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stdhdr.cpp
# ADD CPP /Yc"stdhdr.h"
# End Source File
# Begin Source File

SOURCE=.\src\stripify.cpp
# End Source File
# Begin Source File

SOURCE=.\src\submesh.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\3ds.h
# End Source File
# Begin Source File

SOURCE=.\src\frustum.h
# End Source File
# Begin Source File

SOURCE=.\src\ms3d.h
# End Source File
# Begin Source File

SOURCE=.\src\ms3dread.h
# End Source File
# Begin Source File

SOURCE=.\src\scene.h
# End Source File
# Begin Source File

SOURCE=.\src\scenecamera.h
# End Source File
# Begin Source File

SOURCE=.\src\sceneentity.h
# End Source File
# Begin Source File

SOURCE=.\src\scenelayer.h
# End Source File
# Begin Source File

SOURCE=.\src\stdhdr.h
# End Source File
# Begin Source File

SOURCE=.\src\stripify.h
# End Source File
# End Group
# End Group
# Begin Group ".\include"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\animation.h
# End Source File
# Begin Source File

SOURCE=.\include\enginedll.h
# End Source File
# Begin Source File

SOURCE=.\include\engineguids.h
# End Source File
# Begin Source File

SOURCE=.\include\mesh.h
# End Source File
# Begin Source File

SOURCE=.\include\ray.h
# End Source File
# Begin Source File

SOURCE=.\include\sceneapi.h
# End Source File
# Begin Source File

SOURCE=.\include\sim.h
# End Source File
# Begin Source File

SOURCE=.\include\skeleton.h
# End Source File
# End Group
# End Target
# End Project
