///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techtime.h"
#include "keys.h"
#include "window.h"
#include "connptimpl.h"
#include "filespec.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <zmouse.h>

#include <string>

#include "dbgalloc.h" // must be last header

#ifndef MSH_MOUSEWHEEL
#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
#endif

static uint g_mouseWheelMsg = RegisterWindowMessage(MSH_MOUSEWHEEL);

///////////////////////////////////////////////////////////////////////////////

static HWND CreateDummyWindow()
{
   static const char szDummyWnd[] = "DummyWindow";
   static WNDCLASS dummyWndClass;

   HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	if (!GetClassInfo(hInstance, szDummyWnd, &dummyWndClass))
   {
      memset(&dummyWndClass, 0, sizeof dummyWndClass);
      dummyWndClass.style = 0;
      dummyWndClass.lpfnWndProc = DefWindowProc;
      dummyWndClass.hInstance = hInstance;
      dummyWndClass.lpszClassName = szDummyWnd;
      if (!RegisterClass(&dummyWndClass))
         return NULL;
   }

   // WS_CLIPCHILDREN and WS_CLIPSIBLINGS required by OpenGL
   HWND hDummyWnd = CreateWindowEx(
      0, szDummyWnd, szDummyWnd, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
      NULL, NULL, hInstance, NULL);

   return hDummyWnd;
}

///////////////////////////////////////////////////////////////////////////////
// Create a default OpenGL context using the Win32 function ChoosePixelFormat

static int CreateDefaultContext(HWND hWnd, int * pBpp, HDC * phDC, HGLRC * phGLRC)
{
   Assert(hWnd != NULL);
   Assert(pBpp != NULL);
   Assert(phDC != NULL);
   Assert(phGLRC != NULL);

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
//
// CLASS: cGlContext
//

class cGlContext
{
public:
   cGlContext();
   ~cGlContext();

   bool Create(HWND hWnd, int bpp);
   bool Destroy();

   bool MakeCurrent();

   inline int GetBpp() const { return m_bpp; }
   inline HDC GetHdc() { return m_hDc; }

private:
   HWND m_hWnd;
   int m_bpp;
   HDC m_hDc;
   HGLRC m_hGlrc;
   int m_pixelFormat;
};

///////////////////////////////////////

cGlContext::cGlContext()
 : m_hWnd(NULL),
   m_bpp(0),
   m_hDc(NULL),
   m_hGlrc(NULL),
   m_pixelFormat(0)
{
}

///////////////////////////////////////

cGlContext::~cGlContext()
{
}

///////////////////////////////////////

bool cGlContext::Create(HWND hWnd, int bpp)
{
   if (m_hWnd != NULL || m_pixelFormat != 0 || m_hDc != NULL || m_hGlrc != NULL)
   {
      return false;
   }

   if (!IsWindow(hWnd))
   {
      return false;
   }

   // WS_CLIPCHILDREN and WS_CLIPSIBLINGS required by gl
   DWORD style = GetWindowLong(hWnd, GWL_STYLE);
   style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
   SetWindowLong(hWnd, GWL_STYLE, style);

   m_hWnd = hWnd;
   m_bpp = bpp;

   m_pixelFormat = CreateDefaultContext(m_hWnd, &m_bpp, &m_hDc, &m_hGlrc);

   return (m_pixelFormat != 0);
}

///////////////////////////////////////

bool cGlContext::Destroy()
{
   if (m_hDc != NULL)
   {
      wglMakeCurrent(m_hDc, NULL);
      ReleaseDC(m_hWnd, m_hDc);
      m_hDc = NULL;
   }

   if (m_hGlrc != NULL)
   {
      wglDeleteContext(m_hGlrc);
      m_hGlrc = NULL;
   }

   m_hWnd = NULL;

   return true;
}

///////////////////////////////////////

bool cGlContext::MakeCurrent()
{
   return wglMakeCurrent(m_hDc, m_hGlrc) ? true : false;
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

static const char kFullScreenFormerStyleBits[] = "OldStyleBits";

///////////////////////////////////////

static bool FullScreenBegin(HWND hWnd, int width, int height, int bpp)
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
   Verify(SetProp(hWnd, kFullScreenFormerStyleBits, (HANDLE)styleBits));

   styleBits &= ~(WS_CAPTION | WS_SYSMENU);
   styleBits |= WS_POPUP;

   SetWindowLong(hWnd, GWL_STYLE, styleBits);

   SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, width, height, 0);

   return true;
}

///////////////////////////////////////

static bool FullScreenEnd(HWND hWnd)
{
   if (IsWindow(hWnd))
   {
      // The window's style bits before entering full-screen mode should have
      // been stored as a property on the window.
      HANDLE formerStyleBits = GetProp(hWnd, kFullScreenFormerStyleBits);

      if (formerStyleBits != NULL)
      {
         RemoveProp(hWnd, kFullScreenFormerStyleBits);

         SetWindowLong(hWnd, GWL_STYLE, (long)formerStyleBits);

         SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

         ChangeDisplaySettings(NULL, 0);

         return true;
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWindowWin32
//

class cWindowWin32 : public cComObject2<IMPLEMENTSCP(IWindow, IWindowSink),
                                        IMPLEMENTS(IWindowFullScreen)>
{
public:
   cWindowWin32();

   virtual void OnFinalRelease();

   virtual tResult Create(const sWindowCreateParams * pParams);

   virtual tResult SwapBuffers();

   virtual tResult BeginFullScreen();
   virtual tResult EndFullScreen();

private:
   static std::string GenerateWindowClassName(uint classStyle,
      HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon);

   static std::string RegisterWindowClass(uint classStyle,
      HCURSOR hCursor = NULL, HBRUSH hbrBackground = NULL, HICON hIcon = NULL);

   static bool RegisterWindowClass(const char * pszClassName,
                                   uint classStyle,
                                   HCURSOR hCursor = NULL,
                                   HBRUSH hbrBackground = NULL,
                                   HICON hIcon = NULL);

   static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   ////////////////////////////////////

   HWND GetHwnd() const { return m_hWnd; }

   HWND m_hWnd;

   cGlContext m_context;
};

///////////////////////////////////////

cWindowWin32::cWindowWin32()
 : m_hWnd(NULL)
{
}

///////////////////////////////////////

void cWindowWin32::OnFinalRelease()
{
   FullScreenEnd(m_hWnd);

   m_context.Destroy();

   DestroyWindow(m_hWnd);
   m_hWnd = NULL;
}

///////////////////////////////////////

tResult cWindowWin32::Create(const sWindowCreateParams * pParams)
{
   if (pParams == NULL)
   {
      return E_POINTER;
   }

   if (m_hWnd != NULL)
   {
      DebugMsg("WARNING: Window already created!\n");
      return E_FAIL;
   }

   HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

   std::string wndClassName = RegisterWindowClass(
      CS_HREDRAW | CS_VREDRAW,
      LoadCursor(NULL, IDC_ARROW),
      NULL,
      LoadIcon(hInstance, MAKEINTRESOURCE(1)));

   if (wndClassName.empty())
   {
      return E_FAIL;
   }

   DWORD style = WS_CAPTION | WS_SYSMENU;

   RECT rect;
   SetRect(&rect, 0, 0, pParams->width, pParams->height);
   AdjustWindowRectEx(&rect, style, FALSE, 0);

   m_hWnd = CreateWindowEx(
      0,
      wndClassName.c_str(),
      pParams->title.empty() ? wndClassName.c_str() : pParams->title.c_str(),
      style,
      (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2,
      (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2,
      rect.right - rect.left, rect.bottom - rect.top,
      NULL,
      NULL,
      hInstance,
      this);

   if (m_hWnd == NULL)
   {
      return E_FAIL;
   }

   if ((pParams->flags & kWCF_CreateGlContext) == kWCF_CreateGlContext)
   {
      if (!m_context.Create(m_hWnd, pParams->bpp) || !m_context.MakeCurrent())
      {
         return E_FAIL;
      }
   }

   ShowWindow(m_hWnd, SW_SHOW);
   UpdateWindow(m_hWnd);

   return S_OK;
}

///////////////////////////////////////

tResult cWindowWin32::SwapBuffers()
{
   ::SwapBuffers(m_context.GetHdc());
   return S_OK;
}

///////////////////////////////////////

tResult cWindowWin32::BeginFullScreen()
{
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   if (!FullScreenBegin(m_hWnd, rect.right - rect.left, rect.bottom - rect.top, m_context.GetBpp()))
      return E_FAIL;
   return S_OK;
}

///////////////////////////////////////

tResult cWindowWin32::EndFullScreen()
{
   return FullScreenEnd(m_hWnd) ? S_OK : E_FAIL;
}

///////////////////////////////////////

std::string cWindowWin32::GenerateWindowClassName(uint classStyle,
   HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon)
{
   char szModule[kMaxPath];
   Verify(GetModuleFileName(NULL, szModule, _countof(szModule)));

   strupr(szModule);

   char szWndClassName[200];
   if (hCursor == NULL && hbrBackground == NULL && hIcon == NULL)
   {
      wsprintf(szWndClassName, "%s:%X",
         cFileSpec(szModule).GetFileName(), classStyle);
   }
   else
   {
      wsprintf(szWndClassName, "%s:%X:%X:%X:%X",
         cFileSpec(szModule).GetFileName(), classStyle,
         hCursor, hbrBackground, hIcon);
   }

   return std::string(szWndClassName);
}

///////////////////////////////////////

std::string cWindowWin32::RegisterWindowClass(uint classStyle,
                                              HCURSOR hCursor,
                                              HBRUSH hbrBackground,
                                              HICON hIcon)
{
   std::string wndClassName = GenerateWindowClassName(classStyle, hCursor, hbrBackground, hIcon);

   if (RegisterWindowClass(wndClassName.c_str(), classStyle, hCursor, hbrBackground, hIcon))
   {
      return std::string(wndClassName);
   }
   else
   {
      return std::string("");
   }
}

///////////////////////////////////////

bool cWindowWin32::RegisterWindowClass(const char * pszClassName,
                                       uint classStyle,
                                       HCURSOR hCursor,
                                       HBRUSH hbrBackground,
                                       HICON hIcon)
{
   HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

   WNDCLASS wc;
	if (GetClassInfo(hInstance, pszClassName, &wc))
   {
      Assert(wc.style == classStyle);
      Assert(wc.hCursor == hCursor);
      Assert(wc.hbrBackground == hbrBackground);
      Assert(wc.hIcon == hIcon);
      return true;
   }

   wc.style = classStyle;
   wc.lpfnWndProc = WndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = hIcon;
   wc.hCursor = hCursor;
   wc.hbrBackground = hbrBackground;
   wc.lpszMenuName = NULL;
   wc.lpszClassName = pszClassName;

   if (!RegisterClass(&wc))
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK cWindowWin32::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   double msgTime = TimeGetSecs();

   // if get registered variant of mouse wheel message, handle it as WM_MOUSEWHEEL
   if (message == g_mouseWheelMsg)
   {
      message = WM_MOUSEWHEEL;
   }

   cWindowWin32 * pWnd = (cWindowWin32 *)GetWindowLong(hWnd, GWL_USERDATA);

   switch (message)
   {
      case WM_NCCREATE:
      {
         CREATESTRUCT * pcs = (CREATESTRUCT *)lParam;
         SetWindowLong(hWnd, GWL_USERDATA, (LONG)pcs->lpCreateParams);
         break;
      }

      case WM_DESTROY:
      {
         // The address-of operator here isn't required by the Visual C++
         // compiler, but it is by gcc. It's here to be consistent with
         // the X implementation of IWindow. Plus, I'm guessing that if
         // gcc requires it, it's probably in the C++ spec. MSVC probably 
         // provides the "convenience" of not requiring it.
         pWnd->ForEachConnection(&IWindowSink::OnDestroy, msgTime);
         PostQuitMessage(0);
         return 0;
      }

      case WM_SIZE:
      {
         pWnd->ForEachConnection(&IWindowSink::OnResize, (int)LOWORD(lParam), (int)HIWORD(lParam), msgTime);
         break;
      }

      case WM_PAINT:
      {
         ValidateRect(hWnd, NULL);
         break;
      }

      case WM_ACTIVATEAPP:
      {
         pWnd->ForEachConnection(&IWindowSink::OnActivateApp, wParam ? true : false, msgTime);
         break;
      }

      case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
      {
         long mapped = MapKey(lParam);
         if (mapped != 0)
         {
            pWnd->ForEachConnection(&IWindowSink::OnKeyEvent, mapped, true, msgTime);
         }
         break;
      }

      case WM_SYSCHAR:
      case WM_CHAR:
      {
         long mapped = MapKey(lParam);
         if (mapped == 0)
         {
            pWnd->ForEachConnection(&IWindowSink::OnKeyEvent, (long)wParam, true, msgTime);
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
         pWnd->ForEachConnection(&IWindowSink::OnKeyEvent, mapped, false, msgTime);
         break;
      }

      case WM_MOUSEWHEEL:
      {
         short zDelta = (short)HIWORD(wParam);
         long key = (zDelta < 0) ? kMouseWheelDown : kMouseWheelUp;
         pWnd->ForEachConnection(&IWindowSink::OnKeyEvent, key, true, msgTime);
         pWnd->ForEachConnection(&IWindowSink::OnKeyEvent, key, false, msgTime);
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
         pWnd->ForEachConnection(&IWindowSink::OnMouseEvent, (int)LOWORD(lParam), (int)HIWORD(lParam), mouseState, msgTime);
         break;
      }
   }

   return DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////

IWindow * WindowCreate()
{
   return static_cast<IWindow *>(new cWindowWin32);
}

///////////////////////////////////////

IWindow * WindowCreate(const sWindowCreateParams * pParams)
{
   IWindow * pWnd = WindowCreate();

   if (pWnd != NULL)
   {
      if (FAILED(pWnd->Create(pParams)))
      {
         SafeRelease(pWnd);
         return NULL;
      }
   }

   return pWnd;
}

///////////////////////////////////////////////////////////////////////////////
