# Microsoft Developer Studio Project File - Name="allguids" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=allguids - Win32 StaticDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "allguids.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "allguids.mak" CFG="allguids - Win32 StaticDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "allguids - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "allguids - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "allguids - Win32 Opt" (based on "Win32 (x86) Static Library")
!MESSAGE "allguids - Win32 StaticDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "allguids - Win32 StaticRelease" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "allguids - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build.vc6\Release"
# PROP Intermediate_Dir "..\Build.vc6\Release\allguids"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /I "..\game" /I "..\editor" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "allguids - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\Debug"
# PROP Intermediate_Dir "..\Build.vc6\Debug\allguids"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /I "..\game" /I "..\editor" /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "allguids - Win32 Opt"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "allguids___Win32_Opt"
# PROP BASE Intermediate_Dir "allguids___Win32_Opt"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\Opt"
# PROP Intermediate_Dir "..\Build.vc6\Opt\allguids"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /GX /Zi /O2 /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /I "..\game" /I "..\editor" /D "STRICT" /D "_LIB" /D "OPT" /D "NDEBUG" /D "WIN32" /D "_MBCS" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "allguids - Win32 StaticDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "allguids___Win32_StaticDebug"
# PROP BASE Intermediate_Dir "allguids___Win32_StaticDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Build.vc6\StaticDebug"
# PROP Intermediate_Dir "..\Build.vc6\StaticDebug\allguids"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /I "..\game" /I "..\editor" /D "_DEBUG" /D "STRICT" /D "WIN32" /D "_MBCS" /D "_LIB" /D "STATIC_BUILD" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "allguids - Win32 StaticRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "allguids___Win32_StaticRelease"
# PROP BASE Intermediate_Dir "allguids___Win32_StaticRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Build.vc6\StaticRelease"
# PROP Intermediate_Dir "..\Build.vc6\StaticRelease\allguids"
# PROP Target_Dir ""
F90=df.exe
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\tech\include" /I "..\render\include" /I "..\engine\include" /I "..\game" /I "..\editor" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_MBCS" /D "_LIB" /D "STATIC_BUILD" /FD /c
# SUBTRACT CPP /Fr /YX
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

# Name "allguids - Win32 Release"
# Name "allguids - Win32 Debug"
# Name "allguids - Win32 Opt"
# Name "allguids - Win32 StaticDebug"
# Name "allguids - Win32 StaticRelease"
# Begin Source File

SOURCE=.\allguids.cpp
# End Source File
# End Target
# End Project
