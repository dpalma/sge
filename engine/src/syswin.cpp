///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ggl.h"
#include "gcommon.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>

#include "dbgalloc.h" // must be last header

HINSTANCE g_hInstance = NULL;
bool g_bAppActive = false;

///////////////////////////////////////////////////////////////////////////////

void SysAppActivate(bool active)
{
   g_bAppActive = active;
}

///////////////////////////////////////////////////////////////////////////////

void SysQuit()
{
   PostQuitMessage(0);
}

///////////////////////////////////////////////////////////////////////////////

void SysGetMousePos(int * px, int * py)
{
   POINT cursor;
   GetCursorPos(&cursor);
   ScreenToClient(GetFocus(), &cursor);
   if (px != NULL)
      *px = cursor.x;
   if (py != NULL)
      *py = cursor.y;
}

///////////////////////////////////////////////////////////////////////////////

HANDLE SysGetInstanceHandle()
{
   return g_hInstance;
}

///////////////////////////////////////////////////////////////////////////////

bool SysGetClipboardString(char * psz, int max)
{
   Assert(psz != NULL && max > 0);

   bool bResult = false;

   if (OpenClipboard(NULL))
   {
      HANDLE hData = GetClipboardData(CF_TEXT);

      if (hData != NULL)
      {
         const char * pszData = reinterpret_cast<const char *>(GlobalLock(hData));

         if (pszData != NULL)
         {
            strncpy(psz, pszData, max);
            psz[max - 1] = '\0';

            GlobalUnlock(hData);

            bResult = true;
         }
      }

      Verify(CloseClipboard());
   }

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////

bool SysSetClipboardString(const char * psz)
{
   Assert(psz != NULL);

   bool bResult = false;

   if (OpenClipboard(NULL))
   {
      HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(psz) + 1);

      if (hData != NULL)
      {
         char * pszData = reinterpret_cast<char *>(GlobalLock(hData));

         if (pszData != NULL)
         {
            strcpy(pszData, psz);
            GlobalUnlock(hData);

            if (SetClipboardData(CF_TEXT, hData))
            {
               bResult = true;
            }
         }

         if (!bResult)
         {
            GlobalFree(hData);
         }
      }

      Verify(CloseClipboard());
   }

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd)
{
   g_hInstance = hInstance;

   if (!MainInit(__argc, __argv))
   {
      MainTerm();
      return -1;
   }

   DebugPrintf(NULL, 0, "Vendor:   %s\n", glGetString(GL_VENDOR));
   DebugPrintf(NULL, 0, "Renderer: %s\n", glGetString(GL_RENDERER));
   DebugPrintf(NULL, 0, "Version:  %s\n", glGetString(GL_VERSION));

   MSG msg;

   for (;;)
   {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)
            break;

         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      else
      {
         if (g_bAppActive)
         {
            MainFrame();
         }
         else
         {
            WaitMessage();
         }

         GlShowError();
      }
   }

   MainTerm();

   return msg.wParam;
}

///////////////////////////////////////////////////////////////////////////////
