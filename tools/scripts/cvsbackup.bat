@REM $Id$

@echo off

setlocal

set ARGC=0
:ParseArgs
if "%1"=="" goto ParseArgsDone
for /F "delims=-" %%a in ("%1") do (
   if "%LAST%" == "" (
	   set LAST=%%a
   ) else (
	   if /i "%LAST%" == "cvsrepos" (
		   set CVSREPOS=%%a
	      set /a ARGC+=1
	   ) else if /i "%LAST%" == "dest" (
		   set BACKUPDEST=%%a
	      set /a ARGC+=1
	   ) else (
	      echo Unknown argument %LAST%
	   )
		set LAST=
   )
)
shift
goto ParseArgs
:ParseArgsDone
rem echo Parsed %ARGC% arguments
echo CVS repository %CVSREPOS%
echo Backup to %BACKUPDEST%
if "%ARGC%" == "0" then goto InvalidArgs
if "%CVSREPOS%" == "" then goto NoRepository

if "%BACKUPDEST%" neq "" pushd "%BACKUPDEST%"

set BACKUPFILE=

if exist "\cygwin\bin\nul" and exist "\cygwin\bin\tar.exe" (
	set PATH="\cygwin\bin";%PATH%
	if exist "\cygwin\bin\bzip2.exe" (
		for /L %%d in (1,1,99) do (
			if not exist cvsbackup%%d.tar.bz2 (
				set BACKUPFILE=cvsbackup%%d.tar.bz2
				goto TarBz2Backup
			)
		)
	) else if exist "\cygwin\bin\gzip.exe" (
		for /L %%d in (1,1,99) do (
			if not exist cvsbackup%%d.tar.gz (
				set BACKUPFILE=cvsbackup%%d.tar.gz
				goto TarGzipBackup
			)
		)
	)
)

if exist "..\bin\wzzip.exe" (
	for /L %%d in (1,1,99) do (
		if not exist cvsbackup%%d.zip (
			set BACKUPFILE=cvsbackup%%d.zip
			goto WinZipBackup
		)
	)
)

if exist "..\bin\pkzipc.exe" (
	for /L %%d in (1,1,99) do (
		if not exist cvsbackup%%d.zip (
			set BACKUPFILE=cvsbackup%%d.zip
			goto PkZipBackup
		)
	)
)

goto NoBackupFile

:TarBz2Backup
tar -c %CVSREPOS% | bzip2 >%BACKUPFILE%
goto BackupComplete

:TarGzipBackup
tar -c %CVSREPOS% | gzip >%BACKUPFILE%
goto BackupComplete

:WinZipBackup
..\bin\wzzip -rP %BACKUPFILE% %CVSREPOS%\*.*
goto BackupComplete

:PkZipBackup
..\bin\pkzipc -add -rec -path=full %BACKUPFILE% %CVSREPOS%\*.*
goto BackupComplete

:BackupComplete
if not "%BACKUPDEST%" == "" popd
goto Done

:NoRepository
echo Specify a CVS repository
goto Done

:NoBackupFile
echo Delete some backup files
goto Done

:InvalidArgs
echo Invalid arguments
goto Done

:Done
set BACKUPFILE=
set CVSREPOS=
endlocal
