@REM $Id$

@echo off

if "%1"=="" goto NoRepository

setlocal

set CVSREPOS=%1

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
goto Done

:TarGzipBackup
tar -c %CVSREPOS% | gzip >%BACKUPFILE%
goto Done

:WinZipBackup
..\bin\wzzip -rP %BACKUPFILE% %CVSREPOS%\*.*
goto Done

:PkZipBackup
..\bin\pkzipc -add -rec -path=full %BACKUPFILE% %CVSREPOS%\*.*
goto Done

:NoRepository
echo Specify a CVS repository
goto Done

:NoBackupFile
echo Delete some backup files
goto Done

:Done
set BACKUPFILE=
set CVSREPOS=
endlocal
