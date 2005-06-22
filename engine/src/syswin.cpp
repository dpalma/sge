///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sys.h"

#include "inputapi.h"

#include "keys.h"
#include "globalobj.h"
#include "techtime.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <zmouse.h>

#include <GL/glew.h>

#include <cstdlib>

#include "dbgalloc.h" // must be last header

bool           g_bAppActive = false;

HWND           g_hWnd = NULL;
HDC            g_hDC = NULL;
HGLRC          g_hGLRC = NULL;

void (* g_pfnResizeHack)(int, int) = NULL;

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

   if (OpenClipboard(g_hWnd))
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

      if (!CloseClipboard())
      {
         bResult = false;
      }
   }

   return bResult;
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

LRESULT CALLBACK SysWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   static uint msWheelMsg = RegisterWindowMessage(MSH_MOUSEWHEEL);

   double msgTime = TimeGetSecs();

   // if get registered variant of mouse wheel message, handle it as WM_MOUSEWHEEL
   if (message == msWheelMsg)
   {
      message = WM_MOUSEWHEEL;
   }

   switch (message)
   {
      case WM_DESTROY:
      {
         Assert(hWnd == g_hWnd);

         if (g_hDC != NULL)
         {
            wglMakeCurrent(g_hDC, NULL);
            ReleaseDC(g_hWnd, g_hDC);
            g_hDC = NULL;
         }

         if (g_hGLRC != NULL)
         {
            wglDeleteContext(g_hGLRC);
            g_hGLRC = NULL;
         }

         g_hWnd = NULL;

         PostQuitMessage(0);
         return 0;
      }

      case WM_SIZE:
      {
         if (g_pfnResizeHack != NULL)
         {
            (*g_pfnResizeHack)(LOWORD(lParam), HIWORD(lParam));
         }
         break;
      }

      case WM_PAINT:
      {
         ValidateRect(hWnd, NULL);
         break;
      }

      case WM_ACTIVATEAPP:
      {
         SysAppActivate(wParam ? true : false);
         break;
      }

      case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
      {
         long mapped = MapKey(lParam);
         if (mapped != 0)
         {
            UseGlobal(Input);
            pInput->ReportKeyEvent(mapped, true, msgTime);
         }
         break;
      }

      case WM_SYSCHAR:
      case WM_CHAR:
      {
         long mapped = MapKey(lParam);
         if (mapped == 0)
         {
            UseGlobal(Input);
            pInput->ReportKeyEvent(static_cast<long>(wParam), true, msgTime);
         }
         break;
      }

      case WM_SYSKEYUP:
      case WM_KEYUP:
      {
         long mapped = MapKey(lParam);
         if (mapped == 0)
         {
            mapped = wParam;
         }
         UseGlobal(Input);
         pInput->ReportKeyEvent(mapped, false, msgTime);
         break;
      }

      case WM_MOUSEWHEEL:
      {
         short zDelta = (short)HIWORD(wParam);
         long key = (zDelta < 0) ? kMouseWheelDown : kMouseWheelUp;
         UseGlobal(Input);
         pInput->ReportKeyEvent(key, true, msgTime);
         pInput->ReportKeyEvent(key, false, msgTime);
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
         UseGlobal(Input);
         pInput->ReportMouseEvent((int)LOWORD(lParam), (int)HIWORD(lParam), mouseState, msgTime);
         break;
      }
   }

   return DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
// Create a default OpenGL context using the Win32 function ChoosePixelFormat

static int CreateDefaultContext(HWND hWnd, int * pBpp, HDC * phDC, HGLRC * phGLRC)
{
   *phDC = GetDC(hWnd);
   if (!*phDC)
   {
      return 0;
   }

   if (*pBpp == 0)
   {
      *pBpp = GetDeviceCaps(*phDC, BITSPIXEL);
   }

   PIXELFORMATDESCRIPTOR pfd;
   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = *pBpp;
   pfd.cDepthBits = *pBpp;
   pfd.cStencilBits = *pBpp;
   pfd.dwLayerMask = PFD_MAIN_PLANE;

   int pixelFormat = ChoosePixelFormat(*phDC, &pfd);
   while ((pixelFormat == 0) && (pfd.cStencilBits > 0))
   {
      pfd.cStencilBits /= 2;
      pixelFormat = ChoosePixelFormat(*phDC, &pfd);
   }

   if ((pixelFormat == 0) ||
       !SetPixelFormat(*phDC, pixelFormat, &pfd) ||
       !(*phGLRC = wglCreateContext(*phDC)))
   {
      ReleaseDC(hWnd, *phDC);
      *phDC = NULL;
      *phGLRC = NULL;
      return 0;
   }

   return pixelFormat;
}

///////////////////////////////////////////////////////////////////////////////

HANDLE SysCreateWindow(const tChar * pszTitle, int width, int height)
{
   if (g_hWnd == NULL)
   {
      HINSTANCE hInst = GetModuleHandle(NULL);

      static const char SysWndClass[] = "SYSWNDCLASS";

      WNDCLASS wc;
	   if (GetClassInfo(hInst, SysWndClass, &wc))
      {
         ErrorMsg("Window not created but window class is registered\n");
         return NULL;
      }

      ZeroMemory(&wc, sizeof(wc));
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = SysWndProc;
      wc.hInstance = hInst;
      wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(1));
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.lpszClassName = SysWndClass;
      if (!RegisterClass(&wc))
      {
         ErrorMsg("An error occurred registering the window class\n");
         return NULL;
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
         int bpp = 0;
         if (CreateDefaultContext(g_hWnd, &bpp, &g_hDC, &g_hGLRC) == 0)
         {
            ErrorMsg("An error occurred creating the GL context\n");
            DestroyWindow(g_hWnd);
            g_hWnd = NULL;
            return NULL;
         }

         wglMakeCurrent(g_hDC, g_hGLRC);

         if (glewInit() != GLEW_OK)
         {
            ErrorMsg("GLEW library failed to initialize\n");
            DestroyWindow(g_hWnd);
            g_hWnd = NULL;
            return NULL;
         }

         ShowWindow(g_hWnd, SW_SHOW);
         UpdateWindow(g_hWnd);
      }
   }

   return g_hWnd;
}

///////////////////////////////////////////////////////////////////////////////

void SysSwapBuffers()
{
   SwapBuffers(g_hDC);
}

///////////////////////////////////////////////////////////////////////////////

static const char SysFullScreenOldStyle[] = "OldStyleBits";

bool SysFullScreenBegin(HWND hWnd, int width, int height, int bpp)
{
   if (!IsWindow(hWnd))
   {
      DebugMsg("Valid window handled required to enter full-screen mode\n");
      return false;
   }

   if (bpp == 0)
   {
      HDC hDC = GetDC(NULL);
      if (hDC == NULL)
         return false;
      bpp = GetDeviceCaps(hDC, BITSPIXEL);
      DebugMsg1("Using desktop display depth of %d bpp\n", bpp);
      ReleaseDC(NULL, hDC);
   }

   DEVMODE dm;
   dm.dmSize = sizeof(DEVMODE);
   dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
   dm.dmBitsPerPel = bpp;
   dm.dmPelsWidth = width;
   dm.dmPelsHeight = height;

   if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
   {
      DebugMsg3("Display mode %d x %d, %d bpp not available\n", width, height, bpp);
      return false;
   }

   uint styleBits = GetWindowLong(hWnd, GWL_STYLE);

   // Store the pre-full-screen style bits as a property on the window
   Verify(SetProp(hWnd, SysFullScreenOldStyle, (HANDLE)styleBits));

   styleBits &= ~(WS_CAPTION | WS_SYSMENU);
   styleBits |= WS_POPUP;

   SetWindowLong(hWnd, GWL_STYLE, styleBits);

   SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, width, height, 0);

   return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SysFullScreenEnd(HWND hWnd)
{
   if (IsWindow(hWnd))
   {
      // The window's style bits before entering full-screen mode should have
      // been stored as a property on the window.
      HANDLE formerStyleBits = GetProp(hWnd, SysFullScreenOldStyle);

      if (formerStyleBits != NULL)
      {
         RemoveProp(hWnd, SysFullScreenOldStyle);

         SetWindowLong(hWnd, GWL_STYLE, (long)formerStyleBits);

         SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

         ChangeDisplaySettings(NULL, 0);

         return true;
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

int SysEventLoop(void (* pfnFrameHandler)(), void (* pfnResizeHack)(int, int))
{
   g_pfnResizeHack = pfnResizeHack;

   MSG msg;

   for (;;)
   {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)
         {
            return msg.wParam;
         }

         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      else
      {
         if (g_bAppActive)
         {
            (*pfnFrameHandler)();
         }
         else
         {
            WaitMessage();
         }
      }
   }

   return -1;
}

///////////////////////////////////////////////////////////////////////////////
