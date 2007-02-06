///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "platform/sys.h"
#include "syscommon.h"
#include "platform/keys.h"

#include "tech/configapi.h"
#include "tech/filepath.h"
#include "tech/globalobj.h"
#include "tech/schedulerapi.h"
#include "tech/techtime.h"
#include "tech/threadcallapi.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <zmouse.h>

#include "dll.h"

#include <GL/glew.h>
#include <GL/wglew.h>

#include <cstdlib>

#include "tech/dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(WinKeyEvents);

///////////////////////////////////////////////////////////////////////////////

extern void SysUpdateFrameStats();

bool           g_bAppActive = false;

HWND           g_hWnd = NULL;
HACCEL         g_hAccel = NULL; // For trapping Alt+Enter

static const WORD kAltEnterCommandId = 2006;

typedef tResult (STDCALL * tSHGetFolderPath)(HWND, int, HANDLE, DWORD, LPTSTR);
tSHGetFolderPath g_pfnGetFolderPath = NULL;
cDLL g_shFolder;

///////////////////////////////////////////////////////////////////////////////

void SysAppActivate(bool active)
{
   g_bAppActive = active;
}

///////////////////////////////////////////////////////////////////////////////

void SysQuit()
{
   if (IsWindow(g_hWnd))
   {
      Verify(DestroyWindow(g_hWnd));
   }
   else
   {
      PostQuitMessage(0);
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWindowsClipboard
//
// Helper class for the clipboard functions below. The retries and sleep
// interval are an attempt to fix a problem where the clipboard functions
// fail when Windows Remote Desktop Connection is running. It must be
// periodically clobbering the local clipboard contents with the remote
// contents (a guess).

class cWindowsClipboard
{
public:
   cWindowsClipboard();
   ~cWindowsClipboard();
   tResult Open(HWND hWnd, ulong retries = 1, ulong sleep = 0);
   tResult Close();
   tResult PutString(const tChar * pszString);
   tResult GetString(cStr * pStr, ulong max, ulong retries = 1, ulong sleep = 0);
   DWORD m_lastError;
private:
   bool m_bOpenedClipboard;
};

////////////////////////////////////////

cWindowsClipboard::cWindowsClipboard()
 : m_lastError(NOERROR)
 , m_bOpenedClipboard(false)
{
}

////////////////////////////////////////

cWindowsClipboard::~cWindowsClipboard()
{
   Close();
}

////////////////////////////////////////

tResult cWindowsClipboard::Open(HWND hWnd, ulong retries, ulong sleep)
{
   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   // Impose sanity limits on the number of retries
   if (retries == 0 || retries > 50)
   {
      return E_INVALIDARG;
   }

   // ... and sleep interval
   if (sleep > 500)
   {
      return E_INVALIDARG;
   }

   if (m_bOpenedClipboard)
   {
      return S_FALSE;
   }

   uint iTry = 0;
   while (!m_bOpenedClipboard && (iTry < retries))
   {
      if (OpenClipboard(hWnd))
      {
         m_bOpenedClipboard = true;
         break;
      }
      else
      {
         m_lastError = GetLastError();
      }
      Sleep(sleep);
      ++iTry;
   }

   if (!m_bOpenedClipboard)
   {
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cWindowsClipboard::Close()
{
   if (!m_bOpenedClipboard)
   {
      return S_FALSE;
   }
   if (!CloseClipboard())
   {
      m_lastError = GetLastError();
      return E_FAIL;
   }
   m_bOpenedClipboard = false;
   return S_OK;
}

////////////////////////////////////////

tResult cWindowsClipboard::PutString(const tChar * pszString)
{
   if (pszString == NULL)
   {
      return E_POINTER;
   }

   if (!m_bOpenedClipboard)
   {
      WarnMsg("Clipboard not open\n");
      return E_FAIL;
   }

   size_t memSize = (_tcslen(pszString) + 1) * sizeof(tChar); // plus one for the null terminator

   HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, memSize);
   if (hData == NULL)
   {
      return E_OUTOFMEMORY;
   }

   tChar * pszData = reinterpret_cast<tChar *>(GlobalLock(hData));
   if (pszData == NULL)
   {
      GlobalFree(hData);
      return E_FAIL;
   }

   CopyMemory(pszData, pszString, memSize);
   GlobalUnlock(hData);

#ifdef _UNICODE
   const uint clipFormat = CF_UNICODETEXT;
#else
   const uint clipFormat = CF_TEXT;
#endif

   if (!EmptyClipboard())
   {
      m_lastError = GetLastError();
      GlobalFree(hData);
      return E_FAIL;
   }

   if (!SetClipboardData(clipFormat, hData))
   {
      m_lastError = GetLastError();
      GlobalFree(hData);
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cWindowsClipboard::GetString(cStr * pStr, ulong max, ulong retries, ulong sleep)
{
   // Impose sanity limits on the number of retries
   if (retries == 0 || retries > 50)
   {
      return E_INVALIDARG;
   }

   // ... and sleep interval
   if (sleep > 500)
   {
      return E_INVALIDARG;
   }

#ifdef _UNICODE
   const uint clipFormat = CF_UNICODETEXT;
#else
   const uint clipFormat = CF_TEXT;
#endif

   uint iTry = 0;
   HANDLE hData = GetClipboardData(clipFormat);
   while ((hData == NULL) && (iTry < retries))
   {
      Sleep(sleep);
      hData = GetClipboardData(clipFormat);
      ++iTry;
   }

   if (hData == NULL)
   {
      return E_FAIL;
   }

   const tChar * pszData = reinterpret_cast<const tChar *>(GlobalLock(hData));
   if (pszData == NULL)
   {
      return E_FAIL;
   }

   ulong size = GlobalSize(hData);
   if ((max > 0) && (size > max))
   {
      pStr->assign(pszData, max);
   }
   else
   {
      pStr->assign(pszData);
   }

   GlobalUnlock(hData);

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult SysGetClipboardString(cStr * pStr, ulong max)
{
   if (pStr == NULL)
   {
      return E_POINTER;
   }

#ifdef _UNICODE
   const uint clipFormat = CF_UNICODETEXT;
#else
   const uint clipFormat = CF_TEXT;
#endif

   if (!IsClipboardFormatAvailable(clipFormat))
   {
      return S_FALSE;
   }

   cWindowsClipboard clipboard;
   if (clipboard.Open(g_hWnd) != S_OK)
   {
      ErrorMsg1("Error %d opening clipboard\n", clipboard.m_lastError);
      return E_FAIL;
   }

   if (clipboard.GetString(pStr, max) != S_OK)
   {
      return E_FAIL;
   }

   if (clipboard.Close() != S_OK)
   {
      WarnMsg1("Error %d closing clipboard\n", clipboard.m_lastError);
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult SysSetClipboardString(const tChar * psz)
{
   if (psz == NULL)
   {
      return E_POINTER;
   }

   cWindowsClipboard clipboard;
   if (clipboard.Open(g_hWnd) != S_OK)
   {
      ErrorMsg1("Error %d opening clipboard\n", clipboard.m_lastError);
      return E_FAIL;
   }

   if (clipboard.PutString(psz) != S_OK)
   {
      ErrorMsg1("Error %d putting string data on clipboard\n", clipboard.m_lastError);
      return E_FAIL;
   }

   if (clipboard.Close() != S_OK)
   {
      WarnMsg1("Error %d closing clipboard\n", clipboard.m_lastError);
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

static tResult SysGetPath(int csidl, cFilePath * pPath)
{
   if (pPath == NULL)
   {
      return E_POINTER;
   }

   if (!g_shFolder.IsLoaded())
   {
      if (!g_shFolder.Load(_T("SHFolder.dll")))
      {
         return E_FAIL;
      }

      g_pfnGetFolderPath = reinterpret_cast<tSHGetFolderPath>(
#ifdef _UNICODE
         g_shFolder.GetProcAddress("SHGetFolderPathW"));
#else
         g_shFolder.GetProcAddress("SHGetFolderPathA"));
#endif
   }

   if (g_pfnGetFolderPath != NULL)
   {
      tChar szPath[MAX_PATH];
      ZeroMemory(szPath, sizeof(szPath));

      if ((*g_pfnGetFolderPath)(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, szPath) == S_OK)
      {
         *pPath = cFilePath(szPath);
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult SysGetFontPath(cFilePath * pFontPath)
{
   return SysGetPath(CSIDL_FONTS, pFontPath);
}

///////////////////////////////////////////////////////////////////////////////

tResult SysGetUserPath(cFilePath * pUserPath)
{
   return SysGetPath(CSIDL_PERSONAL, pUserPath);
}

///////////////////////////////////////////////////////////////////////////////

static tResult SysIsRemoteDesktopRunning()
{
   HWND hWndTS = FindWindow(_T("TSSHELLWND"), NULL);
   return IsWindow(hWndTS) ? S_OK : S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

bool SysIsFullScreen(HWND hWnd)
{
   if (IsWindow(hWnd))
   {
      if (GetWindowLongPtr(hWnd, 0) != NULL
         && ((GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST))
      {
         return true;
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

struct sSysWindowedState
{
   long style;
   RECT rect;
};

tResult SysFullScreenBegin(HWND hWnd, int width, int height, int bpp, int hz)
{
   if (!IsWindow(hWnd))
   {
      DebugMsg("Valid window handled required to enter full-screen mode\n");
      return E_INVALIDARG;
   }

   if (SysIsFullScreen(hWnd))
   {
      WarnMsg("Multiple calls to SysFullScreenBegin\n");
      return S_FALSE;
   }

   if (bpp == 0 || hz == 0)
   {
      HDC hDC = GetDC(NULL);
      if (hDC == NULL)
      {
         return E_FAIL;
      }

      if (bpp == 0)
      {
         bpp = GetDeviceCaps(hDC, BITSPIXEL);
      }

      if (hz == 0)
      {
         hz = GetDeviceCaps(hDC, VREFRESH);
      }

      DebugMsg2("Using desktop display depth of %d bpp, refresh rate %d Hz\n", bpp, hz);

      ReleaseDC(NULL, hDC);
   }

   sSysWindowedState * pWindowedState = new sSysWindowedState;
   if (pWindowedState == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pWindowedState->style = GetWindowLongPtr(hWnd, GWL_STYLE);
   if (!GetWindowRect(hWnd, &pWindowedState->rect))
   {
      delete pWindowedState;
      ErrorMsg1("GetWindowRect failed (error %d)\n", GetLastError());
      return E_FAIL;
   }

   Verify(SetWindowLongPtr(hWnd, 0, reinterpret_cast<long_ptr>(pWindowedState)) == 0);

   DEVMODE dm = {0};
   dm.dmSize = sizeof(DEVMODE);
   dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
   dm.dmBitsPerPel = bpp;
   dm.dmPelsWidth = width;
   dm.dmPelsHeight = height;

   if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
   {
      delete pWindowedState;
      ErrorMsg4("Display mode %d x %d, %d bpp, %d Hz not available\n", width, height, bpp, hz);
      return E_FAIL;
   }

   uint styleBits = GetWindowLongPtr(hWnd, GWL_STYLE);

   styleBits &= ~(WS_CAPTION | WS_SYSMENU);
   styleBits |= WS_POPUP;

   SetWindowLongPtr(hWnd, GWL_STYLE, styleBits);

   SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, width, height, SWP_FRAMECHANGED);

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult SysFullScreenEnd(HWND hWnd)
{
   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   sSysWindowedState * pWindowedState = reinterpret_cast<sSysWindowedState *>(SetWindowLongPtr(hWnd, 0, 0));
   if (pWindowedState != NULL)
   {
      ChangeDisplaySettings(NULL, 0);

      SetWindowLongPtr(hWnd, GWL_STYLE, pWindowedState->style);

      SetWindowPos(hWnd, HWND_NOTOPMOST,
         pWindowedState->rect.left, pWindowedState->rect.top,
         pWindowedState->rect.right - pWindowedState->rect.left,
         pWindowedState->rect.bottom - pWindowedState->rect.top,
         SWP_FRAMECHANGED);

      delete pWindowedState;

      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////////////////////////////////////////////
// The characters mapped here should be handled in WM_KEYDOWN, else in WM_CHAR

static const long g_keyMap[128] =
{
   /*   0 */ 0, kEscape, 0, 0, 0, 0, 0, 0,
   /*   8 */ 0, 0, 0, 0, 0, 0, kBackspace, kTab,
   /*  16 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /*  24 */ 0, 0, 0, 0, kEnter, kCtrl, 0, 0,
   /*  32 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /*  40 */ 0, 0, kLShift, 0, 0, 0, 0, 0,
   /*  48 */ 0, 0, 0, 0, 0, 0, kRShift, 0,
   /*  56 */ kAlt, kSpace, 0, kF1, kF2, kF3, kF4, kF5,
   /*  64 */ kF6, kF7, kF8, kF9, kF10, kPause, 0, kHome,
   /*  72 */ kUp, kPageUp, 0, kLeft, 0, kRight, 0, kEnd,
   /*  80 */ kDown, kPageDown, kInsert, kDelete, 0, 0, 0, kF11,
   /*  88 */ kF12, 0, 0, 0, 0, 0, 0, 0,
   /*  96 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /* 104 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /* 112 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /* 120 */ 0, 0, 0, 0, 0, 0, 0, 0,
};

///////////////////////////////////////////////////////////////////////////////
// The keydata parameter is the LPARAM from a Windows key message

static long MapKey(long keydata)
{
   //int repeatCount = keydata & 0xFFFF;

   int scanCode = (keydata >> 16) & 0xFF;

   //bool isExtended = false;
   //if (keydata & (1 << 24))
   //   isExtended = true;

   return g_keyMap[scanCode];
}

///////////////////////////////////////////////////////////////////////////////

bool SysHandleWindowsMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   static uint msWheelMsg = RegisterWindowMessage(MSH_MOUSEWHEEL);

   double msgTime = TimeGetSecs();

   // if get registered variant of mouse wheel message, handle it as WM_MOUSEWHEEL
   if (message == msWheelMsg)
   {
      message = WM_MOUSEWHEEL;
   }

   bool bHandled = false;

   switch (message)
   {
      case WM_DESTROY:
      {
         if (g_pfnDestroyCallback != NULL)
         {
            (*g_pfnDestroyCallback)();
         }

         if (g_hAccel)
         {
            DestroyAcceleratorTable(g_hAccel);
            g_hAccel = NULL;
         }

         if (hWnd == g_hWnd)
         {
            g_hWnd = NULL;
         }

         SysFullScreenEnd(hWnd);
         PostQuitMessage(0);
         bHandled = true;
         break;
      }

      case WM_PAINT:
      {
         if (hWnd == g_hWnd)
         {
            ValidateRect(hWnd, NULL);
            bHandled = true;
         }
         break;
      }

      case WM_ACTIVATEAPP:
      {
         SysAppActivate(wParam ? true : false);
         bHandled = true;
         break;
      }

      case WM_SIZE:
      {
         if (g_pfnResizeCallback != NULL)
         {
            (*g_pfnResizeCallback)(LOWORD(lParam), HIWORD(lParam), msgTime);
            bHandled = true;
         }
         break;
      }

      case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
      {
         DebugMsgEx2(WinKeyEvents, "WM_KEYDOWN %d %d\n", wParam, lParam);
         long mapped = MapKey(lParam);
         if (mapped != 0)
         {
            if (g_pfnKeyCallback != NULL)
            {
               (*g_pfnKeyCallback)(mapped, true, msgTime, g_keyCallbackUserData);
               bHandled = true;
            }
         }
         break;
      }

      case WM_SYSCHAR:
      case WM_CHAR:
      {
         DebugMsgEx2(WinKeyEvents, "WM_CHAR %d %d\n", wParam, lParam);
         long mapped = MapKey(lParam);
         if (g_pfnCharCallback != NULL)
         {
            (*g_pfnCharCallback)(static_cast<tChar>(wParam), msgTime, g_charCallbackUserData);
            bHandled = true;
         }
         break;
      }

      case WM_SYSKEYUP:
      case WM_KEYUP:
      {
         DebugMsgEx2(WinKeyEvents, "WM_KEYUP %d %d\n", wParam, lParam);
         long mapped = MapKey(lParam);
         if (mapped != 0)
         {
            if (g_pfnKeyCallback != NULL)
            {
               (*g_pfnKeyCallback)(mapped, false, msgTime, g_keyCallbackUserData);
               bHandled = true;
            }
         }
         break;
      }

      case WM_COMMAND:
      {
         if (LOWORD(wParam) == kAltEnterCommandId)
         {
            if (SysIsFullScreen(hWnd))
            {
               SysFullScreenEnd(hWnd);
            }
            else
            {
               RECT rect;
               GetClientRect(hWnd, &rect);
               SysFullScreenBegin(hWnd, rect.right, rect.bottom, 0, 0);
            }
         }
         break;
      }

      case WM_MOUSEWHEEL:
      {
         short zDelta = (short)HIWORD(wParam);
         long key = (zDelta < 0) ? kMouseWheelDown : kMouseWheelUp;
         if (g_pfnKeyCallback != NULL)
         {
            (*g_pfnKeyCallback)(key, true, msgTime, g_keyCallbackUserData);
            (*g_pfnKeyCallback)(key, false, msgTime, g_keyCallbackUserData);
            bHandled = true;
         }
         break;
      }

      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_MOUSEMOVE:
      {
         uint mouseState = 0;
         if (wParam & MK_LBUTTON)
            mouseState |= kLMouseDown;
         if (wParam & MK_RBUTTON)
            mouseState |= kRMouseDown;
         if (wParam & MK_MBUTTON)
            mouseState |= kMMouseDown;
         tSysMouseEventFn pfnMouse = (message == WM_MOUSEMOVE) ? g_pfnMouseMoveCallback : g_pfnMouseCallback;
         uint_ptr userData = (message == WM_MOUSEMOVE) ? g_mouseMoveCallbackUserData : g_mouseCallbackUserData;
         if (pfnMouse != NULL)
         {
            (*pfnMouse)(static_cast<int>(LOWORD(lParam)),
                        static_cast<int>(HIWORD(lParam)),
                        mouseState, msgTime, userData);
            bHandled = true;
         }
         break;
      }
   }

   return bHandled;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK SysWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   SysHandleWindowsMessage(hWnd, message, wParam, lParam);
   return DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////

tResult SysCreateWindow(const tChar * pszTitle, int width, int height)
{
   if (g_hWnd == NULL)
   {
      HINSTANCE hInst = GetModuleHandle(NULL);

      static const char SysWndClass[] = "SYSWNDCLASS";

      WNDCLASS wc;
	   if (GetClassInfo(hInst, SysWndClass, &wc))
      {
         ErrorMsg("Window not created but window class is registered\n");
         return E_FAIL;
      }

      ZeroMemory(&wc, sizeof(wc));
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = SysWndProc;
      wc.hInstance = hInst;
      wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(1));
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.lpszClassName = SysWndClass;
      wc.cbWndExtra = sizeof(void*);
      if (!RegisterClass(&wc))
      {
         ErrorMsg("An error occurred registering the window class\n");
         return E_FAIL;
      }

      // WS_CLIPCHILDREN and WS_CLIPSIBLINGS required by GL
      static const DWORD SysWndStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

      RECT rect;
      SetRect(&rect, 0, 0, width, height);
      AdjustWindowRectEx(&rect, SysWndStyle, FALSE, 0);

      g_hWnd = CreateWindowEx(0, SysWndClass, pszTitle, SysWndStyle,
         (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2,
         (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2,
         rect.right - rect.left, rect.bottom - rect.top,
         NULL, NULL, hInst, NULL);

      if (g_hWnd != NULL)
      {
         Assert(g_hAccel == NULL);
         ACCEL accel = {0};
         accel.fVirt = FALT | FNOINVERT | FVIRTKEY;
         accel.cmd = kAltEnterCommandId;
         accel.key = VK_RETURN;
         g_hAccel = CreateAcceleratorTable(&accel, 1);

         ShowWindow(g_hWnd, SW_SHOW);
         UpdateWindow(g_hWnd);
         return S_OK;
      }
   }
   else
   {
      return S_FALSE;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

HWND SysGetMainWindow()
{
   return g_hWnd;
}

///////////////////////////////////////////////////////////////////////////////

tResult SysGetWindowSize(int * pWidth, int * pHeight)
{
   if (pWidth == NULL || pHeight == NULL)
   {
      return E_POINTER;
   }
   if (IsWindow(g_hWnd))
   {
      RECT rect = {0};
      if (GetClientRect(g_hWnd, &rect))
      {
         *pWidth = (rect.right - rect.left);
         *pHeight = (rect.bottom - rect.top);
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

// Calls to SysEventLoop may be nested to implement modal dialog loops
// and such.  Quit messages must re-posted from nested calls for the app
// to exit properly.
static int g_eventLoopCalls = 0;

int SysEventLoop(tSysFrameFn pfnFrameHandler, uint flags)
{
   MSG msg;
   int result = -1;

   g_eventLoopCalls++;

   UseGlobal(Scheduler);
   UseGlobal(ThreadCaller);

   bool bRunScheduler = ((flags & kSELF_RunScheduler) == kSELF_RunScheduler);
   bool bReceiveThreadCalls = ((flags & kSELF_ReceiveThreadCalls) == kSELF_ReceiveThreadCalls);

   for (;;)
   {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)
         {
            if (g_eventLoopCalls > 1)
            {
               // Re-post the WM_QUIT for the main message loop to get it
               PostQuitMessage(msg.wParam);
            }
            result = msg.wParam;
            goto LExit;
         }

         if (!TranslateAccelerator(g_hWnd, g_hAccel, &msg))
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
      }
      else
      {
         SysUpdateFrameStats();

         if (g_bAppActive)
         {
            if (bRunScheduler)
            {
               pScheduler->NextFrame();
            }

            if (bReceiveThreadCalls)
            {
               pThreadCaller->ReceiveCalls(NULL);
            }

            if (pfnFrameHandler != NULL)
            {
               if ((*pfnFrameHandler)() != S_OK)
               {
                  result = 0;
                  goto LExit;
               }
            }
            else if (g_pfnFrameCallback != NULL)
            {
               if ((*g_pfnFrameCallback)() != S_OK)
               {
                  result = 0;
                  goto LExit;
               }
            }
         }
         else
         {
            WaitMessage();
         }
      }
   }

LExit:
   g_eventLoopCalls--;
   return result;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

TEST(Clipboard)
{
   if (SysIsRemoteDesktopRunning() == S_OK)
   {
      WarnMsg("Skipping clipboard tests because \"Remote Desktop Connection\" is running\n");
   }
   else
   {
      static const ulong kInSize = 20;
      static const ulong kTruncSize = 5;

#ifdef _UNICODE
      std::wstring in(kInSize, _T('X'));
#else
      std::string in(kInSize, 'X');
#endif

      std::string original;
      CHECK(SUCCEEDED(SysGetClipboardString(&original, 0))); // S_OK or S_FALSE results are both OK

      CHECK(SysSetClipboardString(in.c_str()) == S_OK);

      cStr out("garbage should be cleared/over-written");
      CHECK(SysGetClipboardString(&out) == S_OK);
      CHECK_EQUAL(out.length(), in.length());
      CHECK(out.compare(in) == 0);

      CHECK(SysGetClipboardString(&out, kTruncSize) == S_OK);
#ifdef _UNICODE
      CHECK(out.compare(std::wstring(kTruncSize, _T('X'))) == 0);
#else
      CHECK(out.compare(std::string(kTruncSize, 'X')) == 0);
#endif

      CHECK(SysSetClipboardString(original.c_str()) == S_OK);
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
