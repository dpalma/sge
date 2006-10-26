///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include <cstdio>
#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOGDI
#include <windows.h>
#include <crtdbg.h>
#endif

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
// Shows the user an assertion failure dialog box. Returns true if
// the user wants to break into the debugger.

bool AssertFail(const tChar * pszFile, int line,
                const tChar * pszExpr,
                const tChar * pszMsg)
{
   bool bWantDbgBreak = false;

   tChar szMsg[1024];
   if (pszMsg != NULL)
   {
      _sntprintf(szMsg, _countof(szMsg), _T("%s: %s"), pszExpr, pszMsg);
   }
   else
   {
      _sntprintf(szMsg, _countof(szMsg), _T("%s"), pszExpr);
   }

   techlog.Print(pszFile, line, kError, _T("ASSERTION FAILURE: %s\n"), szMsg);

#ifdef _MSC_VER
   // Remove WM_QUIT because if it's in the queue then the message
   // box won't display
   MSG msg;
   BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);

#ifdef _DEBUG
#ifdef _UNICODE
   size_t l = WideCharToMultiByte(CP_ACP, 0, pszFile, _tcslen(pszFile), NULL, 0, NULL, NULL);
   char * pszFileA = reinterpret_cast<char*>(alloca(sizeof(char) * (l + 1)));
   WideCharToMultiByte(CP_ACP, 0, pszFile, _tcslen(pszFile), pszFileA, l, NULL, NULL);
   l = WideCharToMultiByte(CP_ACP, 0, szMsg, _tcslen(szMsg), NULL, 0, NULL, NULL);
   char * pszMsgA = reinterpret_cast<char*>(alloca(sizeof(char) * (l + 1)));
   WideCharToMultiByte(CP_ACP, 0, szMsg, _tcslen(szMsg), pszMsgA, l, NULL, NULL);
#else
   const char * pszFileA = pszFile;
   const char * pszMsgA = szMsg;
#endif
   if (_CrtDbgReport(_CRT_ASSERT, pszFileA, line, NULL, pszMsgA))
   {
      bWantDbgBreak = true;
   }
#else
   tChar szProgram[MAX_PATH], szBuffer[1024];
   if (!GetModuleFileName(NULL, szProgram, _countof(szProgram)))
   {
      lstrcpy(szProgram, _T("<program name unknown>"));
   }
   _sntprintf(szBuffer, _countof(szBuffer),
      _T("Program: %s\nFile: %s\nLine: %d\nExpression: %s\n\nBreak into debugger?"),
      szProgram, pszFile, line, szMsg);
   if (MessageBox(NULL, szBuffer, _T("Assertion Failure"), MB_ICONSTOP | MB_YESNO) == IDYES)
   {
      bWantDbgBreak = true;
   }
#endif

   if (bQuit)
   {
      PostQuitMessage(msg.wParam);
   }
#endif // _MSC_VER

   return bWantDbgBreak;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
struct sDebugState
{
   sDebugState()
   {
      // Use the debug allocator and dump all memory leaks at exit
      int crtDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
      _CrtSetDbgFlag(crtDbgFlag | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
   }
} g_debugState;
#endif

///////////////////////////////////////////////////////////////////////////////
