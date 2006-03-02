///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sys.h"

#include "keys.h"
#include "globalobj.h"
#include "techtime.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <zmouse.h>

#include <GL/glew.h>

#if HAVE_DIRECTX
#include <d3d9.h>
#endif

#include <cstdlib>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool           g_bAppActive = false;

HWND           g_hWnd = NULL;
HDC            g_hDC = NULL;
HGLRC          g_hGLRC = NULL;

#if HAVE_DIRECTX
HMODULE                       g_hD3D9 = NULL;
cAutoIPtr<IDirect3D9>         g_pDirect3D9;
cAutoIPtr<IDirect3DDevice9>   g_pDirect3DDevice9;
#endif

// from syscommon.cpp
extern tSysKeyEventFn   g_pfnKeyCallback;
extern tSysMouseEventFn g_pfnMouseCallback;
extern tSysFrameFn      g_pfnFrameCallback;
extern tSysResizeFn     g_pfnResizeCallback;
extern uint_ptr         g_keyCallbackUserData;
extern uint_ptr         g_mouseCallbackUserData;
extern void SysUpdateFrameStats();


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

static tResult SysIsRemoteDesktopRunning()
{
   HWND hWndTS = FindWindow(_T("TSSHELLWND"), NULL);
   return IsWindow(hWndTS) ? S_OK : S_FALSE;
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

#if HAVE_DIRECTX
         SafeRelease(g_pDirect3DDevice9);
         SafeRelease(g_pDirect3D9);

         if (g_hD3D9 != NULL)
         {
            FreeLibrary(g_hD3D9);
            g_hD3D9 = NULL;
         }
#endif

         if (hWnd == g_hWnd)
         {
            g_hWnd = NULL;
         }

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
         }
         bHandled = true;
         break;
      }

      case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
      {
         long mapped = MapKey(lParam);
         if (mapped != 0)
         {
            if (g_pfnKeyCallback != NULL)
            {
               (*g_pfnKeyCallback)(mapped, true, msgTime, g_keyCallbackUserData);
            }
         }
         bHandled = true;
         break;
      }

      case WM_SYSCHAR:
      case WM_CHAR:
      {
         long mapped = MapKey(lParam);
         if (mapped == 0)
         {
            if (g_pfnKeyCallback != NULL)
            {
               (*g_pfnKeyCallback)(static_cast<long>(wParam), true, msgTime, g_keyCallbackUserData);
            }
         }
         bHandled = true;
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
         if (g_pfnKeyCallback != NULL)
         {
            (*g_pfnKeyCallback)(mapped, false, msgTime, g_keyCallbackUserData);
         }
         bHandled = true;
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
         }
         bHandled = true;
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
         if (g_pfnMouseCallback != NULL)
         {
            (*g_pfnMouseCallback)(static_cast<int>(LOWORD(lParam)),
                                  static_cast<int>(HIWORD(lParam)),
                                  mouseState, msgTime, g_mouseCallbackUserData);
         }
         bHandled = true;
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

#if HAVE_DIRECTX
static tResult InitDirect3D9(HWND hWnd, IDirect3D9 * * ppD3d, IDirect3DDevice9 * * ppDevice)
{
   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   if (ppD3d == NULL || ppDevice == NULL)
   {
      return E_POINTER;
   }

   if (g_hD3D9 == NULL)
   {
      g_hD3D9 = LoadLibrary("d3d9.dll");
      if (g_hD3D9 == NULL)
      {
         return E_FAIL;
      }
   }

   typedef IDirect3D9 * (WINAPI * tDirect3DCreate9Fn)(UINT);
   tDirect3DCreate9Fn pfnDirect3DCreate9 = reinterpret_cast<tDirect3DCreate9Fn>(GetProcAddress(g_hD3D9, "Direct3DCreate9"));
   if (pfnDirect3DCreate9 == NULL)
   {
      return E_FAIL;
   }

   cAutoIPtr<IDirect3D9> pD3d((*pfnDirect3DCreate9)(D3D_SDK_VERSION));
   if (!pD3d)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;
   cAutoIPtr<IDirect3DDevice9> pD3dDevice;

   D3DDISPLAYMODE displayMode;
   if (pD3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode) == D3D_OK)
   {
      D3DPRESENT_PARAMETERS presentParams;
      memset(&presentParams, 0, sizeof(presentParams));
      presentParams.BackBufferCount = 1;
      presentParams.BackBufferFormat = displayMode.Format;
      presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
      presentParams.Windowed = TRUE;
      presentParams.EnableAutoDepthStencil = TRUE;
      presentParams.AutoDepthStencilFormat = D3DFMT_D16;
      presentParams.hDeviceWindow = hWnd;
      presentParams.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL | D3DPRESENTFLAG_DEVICECLIP;

      result = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
         D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pD3dDevice);
      if (FAILED(result))
      {
         // Try reference device if failed to create hardware device
         result = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pD3dDevice);
      }
   }

   if (FAILED(result))
   {
      ErrorMsg1("D3D error %x\n", result);
   }
   else
   {
      *ppD3d = CTAddRef(pD3d);
      *ppDevice = CTAddRef(pD3dDevice);
   }

   return result;
}
#endif // HAVE_DIRECTX

///////////////////////////////////////////////////////////////////////////////

HANDLE SysCreateWindow(const tChar * pszTitle, int width, int height, eSys3DAPI api)
{
   AssertMsg(api == kOpenGL || api == kDirect3D9, "New 3D API added to enumerated type?");

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
         if (api == kOpenGL)
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
         }
#if HAVE_DIRECTX
         else if (api == kDirect3D9)
         {
            if (InitDirect3D9(g_hWnd, &g_pDirect3D9, &g_pDirect3DDevice9) != D3D_OK)
            {
               ErrorMsg("Direct3D failed to initialize\n");
               DestroyWindow(g_hWnd);
               g_hWnd = NULL;
               return NULL;
            }
         }
#endif
         else
         {
            ErrorMsg1("Unknown 3D API %d\n", static_cast<int>(api));
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

tResult SysGetDirect3DDevice9(IDirect3DDevice9 * * ppDevice)
{
#if HAVE_DIRECTX
   return g_pDirect3DDevice9.GetPointer(ppDevice);
#else
   return E_FAIL;
#endif
}

///////////////////////////////////////////////////////////////////////////////

void SysSwapBuffers()
{
   if (g_hDC != NULL)
   {
      Verify(SwapBuffers(g_hDC));
   }
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

// Calls to SysEventLoop may be nested to implement modal dialog loops
// and such.  Quit messages must re-posted from nested calls for the app
// to exit properly.
static int g_eventLoopCalls = 0;

int SysEventLoop(tSysFrameFn pfnFrameHandler)
{
   MSG msg;
   int result = -1;

   g_eventLoopCalls++;

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

         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      else
      {
         SysUpdateFrameStats();

         if (g_bAppActive)
         {
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

#ifdef HAVE_CPPUNITLITE2

TEST(TestClipboard)
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

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
