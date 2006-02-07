/* $Id$
 *
 * A lib.exe replacement for Microsoft Visual C++ Toolkit 2003
 */

#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

static const TCHAR szLibFlag[] = _T(" /lib ");

int main(int arg, char *argv[])
{
  BOOL bResult = FALSE;
  STARTUPINFO startupInfo = {0};
  PROCESS_INFORMATION processInfo = {0};
  LPTSTR lpCmdLine = GetCommandLine(), lpNewCmdLine, lpTemp;
  TCHAR szLink[_MAX_PATH];
  int cmdLineLen = lstrlen(lpCmdLine);
  int i = _tcscspn(lpCmdLine, _T(" \t"));
  if (i < cmdLineLen)
    lpCmdLine += i;
  lpNewCmdLine = (LPTSTR)malloc(lstrlen(lpCmdLine)
				       + lstrlen(szLibFlag));
  if (lpNewCmdLine == NULL)
    goto LExit;
  sprintf(lpNewCmdLine, "%s%s", szLibFlag, lpCmdLine);
  if (!GetModuleFileName(NULL, szLink, sizeof(szLink)/sizeof(szLink[0])))
    goto LExit;
  lpTemp = _tcsrchr(szLink, _T('\\'));
  if (lpTemp != NULL)
    lstrcpy(lpTemp + 1, _T("link.exe"));
  bResult = CreateProcess(szLink,
			  lpNewCmdLine,
			  NULL,
			  NULL,
			  FALSE,
			  DETACHED_PROCESS,
			  NULL,
			  NULL,
			  &startupInfo,
			  &processInfo);
 LExit:
  if (processInfo.hProcess != NULL)
    CloseHandle(processInfo.hProcess);
  if (processInfo.hThread != NULL)
    CloseHandle(processInfo.hThread);
  if (lpNewCmdLine != NULL)
    free(lpNewCmdLine);
  return bResult ? EXIT_SUCCESS : EXIT_FAILURE;
}
