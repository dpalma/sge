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
echo Backing up to %BACKUPDEST%
if "%ARGC%" == "0" then goto InvalidArgs
if "%CVSREPOS%" == "" then goto NoRepository

if "%BACKUPDEST%" neq "" pushd "%BACKUPDEST%"

if defined DATE (
	:: When defined, the DATE environment variable looks like "Day mm/dd/yyyy"
	for /f "tokens=1,2,3,4 delims=/ " %%a in ("%DATE%") do (
		set BACKUPNAME=cvsbackup-%%d-%%b-%%c
	)
) else (
	set BACKUPNAME=cvsbackup
)
set BACKUPFILE=

if exist "\cygwin\bin\nul" and exist "\cygwin\bin\tar.exe" (
	set PATH="\cygwin\bin";%PATH%
	if exist "\cygwin\bin\bzip2.exe" (
		set BACKUPFILE=%BACKUPNAME%.tar.bz2
		if exist %BACKUPNAME%.tar.bz2 (
			for /L %%d in (99,-1,1) do (
				if not exist %BACKUPNAME%-#%%d.tar.bz2 (
					set BACKUPFILE=%BACKUPNAME%-#%%d.tar.bz2
				)
			)
		)
		goto TarBz2Backup
	) else if exist "\cygwin\bin\gzip.exe" (
		set BACKUPFILE=%BACKUPNAME%.tar.gz
		if exist %BACKUPNAME%.tar.gz (
			for /L %%d in (99,-1,1) do (
				if not exist %BACKUPNAME%-#%%d.tar.gz (
					set BACKUPFILE=%BACKUPNAME%-#%%d.tar.gz
				)
			)
		)
		goto TarGzipBackup
	)
)

if exist "..\bin\wzzip.exe" (
	set BACKUPFILE=%BACKUPNAME%.zip
	if exist %BACKUPNAME%.zip (
		for /L %%d in (99,-1,1) do (
			if not exist %BACKUPNAME%-#%%d.zip (
				set BACKUPFILE=%BACKUPNAME%-#%%d.zip
			)
		)
	)
	goto WinZipBackup
)

if exist "..\bin\pkzipc.exe" (
	set BACKUPFILE=%BACKUPNAME%.zip
	if exist %BACKUPNAME%.zip (
		for /L %%d in (99,-1,1) do (
			if not exist %BACKUPNAME%-#%%d.zip (
				set BACKUPFILE=%BACKUPNAME%-#%%d.zip
			)
		)
	)
	goto PkZipBackup
)

goto NoBackupFile

:TarBz2Backup
echo Backup file is "%BACKUPFILE%"
if "%BACKUPFILE%" == "" goto NoBackupFile
tar -c %CVSREPOS% | bzip2 >%BACKUPFILE%
goto BackupComplete

:TarGzipBackup
if "%BACKUPFILE%" == "" goto NoBackupFile
tar -c %CVSREPOS% | gzip >%BACKUPFILE%
goto BackupComplete

:WinZipBackup
if "%BACKUPFILE%" == "" goto NoBackupFile
..\bin\wzzip -rP %BACKUPFILE% %CVSREPOS%\*.*
goto BackupComplete

:PkZipBackup
if "%BACKUPFILE%" == "" goto NoBackupFile
..\bin\pkzipc -add -rec -path=full %BACKUPFILE% %CVSREPOS%\*.*
goto BackupComplete

:BackupComplete
if not "%BACKUPDEST%" == "" popd
goto Done

:NoRepository
echo Specify a CVS repository
goto Done

:NoBackupFile
echo Unable to determine an appropriate backup file name
goto Done

:InvalidArgs
echo Invalid arguments
goto Done

:Done
set BACKUPNAME=
set BACKUPFILE=
set CVSREPOS=
endlocal
