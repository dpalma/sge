// Windows Template Library - WTL version 7.1
// Copyright (C) 1997-2003 Microsoft Corporation
// All rights reserved.
//
// This file is a part of the Windows Template Library.
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.

// Setup program for the WTL App Wizard for EVC++ 4.0

main();

function main()
{
	var bDebug = false;
	var Args = WScript.Arguments;
	if(Args.length > 0 && Args(0) == "/debug")
		bDebug = true;

	// Create shell object
	var WSShell = WScript.CreateObject("WScript.Shell");
	// Create file system object
	var FileSys = WScript.CreateObject("Scripting.FileSystemObject");

	var strValue = FileSys.GetAbsolutePathName(".");
	if(strValue == null || strValue == "")
		strValue = ".";

	var strSourceFolder = strValue;
	if(bDebug)
		WScript.Echo("Source: " + strSourceFolder);

	var strEVC4Key = "HKLM\\Software\\Microsoft\\CEStudio\\4.0\\Setup\\CECommonDir";
	try
	{
		strValue = WSShell.RegRead(strEVC4Key);
	}
	catch(e)
	{
		WScript.Echo("ERROR: Cannot find where Embedded Visual C++ 4.0 is installed.");
		return;
	}

	var strDestFolder = strValue + "\\EVC\\Template";
	if(bDebug)
		WScript.Echo("Destination: " + strDestFolder);
	if(!FileSys.FolderExists(strDestFolder))
	{
		WScript.Echo("ERROR: Cannot find destination folder (should be: " + strDestFolder + ")");
		return;
	}

	// Copy App Wizard file
	try
	{
		var strSrc = strSourceFolder + "\\AtlAppCE.awx";
		var strDest = strDestFolder + "\\";
		FileSys.CopyFile(strSrc, strDest);
	}
	catch(e)
	{
		var strError = "no info";
		if(e.description.length != 0)
			strError = e.description;
		WScript.Echo("ERROR: Cannot copy file (" + strError + ")");
		return;
	}

	WScript.Echo("App Wizard successfully installed!");
}
