///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef _DEBUG // (entire file)

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOGDI
#include <windows.h>
#include <crtdbg.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
// Shows the user an assertion failure dialog box. Returns true if
// the user wants to break into the debugger.

bool AssertFail(const char * pszFile, int line, const char * pszExpr)
{
#ifdef _MSC_VER
   // remove WM_QUIT because if it's in the queue then the message
   // box won't display
   MSG msg;
   BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
   bool bResult = _CrtDbgReport(_CRT_ASSERT, pszFile, line, NULL, pszExpr)
      ? true : false;
   if (bQuit)
      PostQuitMessage(msg.wParam);
#else
   bool bResult = false;
   techlog.Print(pszFile, line, kError, "ASSERTION FAILURE: %s\n", pszExpr);
#endif
   return bResult;
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

#endif // _DEBUG (entire file)
