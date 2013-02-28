###############################################################################
# $Id$

XC=xcopy.exe
XCFLAGS=/d /y /q
RM=del

.SILENT:

ALL:
   @@echo Copying Cg DLLs to $(OUTDIR)
   @@if not exist $(OUTDIR)\nul mkdir $(OUTDIR)
   $(XC) .\..\..\3rdparty\Cg\bin\cg.dll $(OUTDIR) $(XCFLAGS) >nul
   $(XC) .\..\..\3rdparty\Cg\bin\cgD3D8.dll $(OUTDIR) $(XCFLAGS) >nul
   $(XC) .\..\..\3rdparty\Cg\bin\cgD3D9.dll $(OUTDIR) $(XCFLAGS) >nul
   $(XC) .\..\..\3rdparty\Cg\bin\cgGL.dll $(OUTDIR) $(XCFLAGS) >nul

CLEAN:
   @@echo Deleting Cg DLLs in $(OUTDIR)
   $(RM) $(OUTDIR)\cg*.dll >nul 2>nul
