///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "window.h"
#include "keys.h"
#include "techtime.h"
#include "connptimpl.h"

#include <map>
#include <locale>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "dbgalloc.h" // must be last header

Display * g_pXDisplay = NULL;

///////////////////////////////////////////////////////////////////////////////

typedef std::map<Window, IWindowX11 *> tWindowMap;

tWindowMap g_windows;

///////////////////////////////////////////////////////////////////////////////
// These functions do NOT AddRef the IWindowX11 interface pointer because
// the implementation behind IWindowX11 calls AddWindow when a window
// object is created and RemoveWindow when it is destroyed. So, every pointer
// in the map is always valid.

bool RegisterWindow(Window window, IWindowX11 * pWindow)
{
   Assert(window != 0);
   Assert(pWindow != NULL);
   std::pair<tWindowMap::iterator, bool> result = g_windows.insert(std::make_pair(window, pWindow));
   return result.second;
}

IWindowX11 * RevokeWindow(Window window)
{
   tWindowMap::iterator iter = g_windows.find(window);
   if (iter == g_windows.end())
      return NULL;
   IWindowX11 * result = iter->second;
   g_windows.erase(iter);
   if (g_windows.empty())
   {
      XCloseDisplay(g_pXDisplay);
      g_pXDisplay = NULL;
   }
   return result;
}

IWindowX11 * FindWindow(Window window)
{
   tWindowMap::iterator iter = g_windows.find(window);
   if (iter == g_windows.end())
      return NULL;
   iter->second->AddRef();
   return iter->second;
}

///////////////////////////////////////////////////////////////////////////////

static bool MapXKeysym(KeySym keysym, long * pkeycode)
{
   static const struct { long sym; long code; } sym2code[] =
   {
      { XK_KP_Tab, kTab }, { XK_Tab, kTab },
      { XK_Return, kEnter }, { XK_KP_Enter, kEnter }, // ??? XK_Linefeed
      { XK_KP_Space, kSpace },
      { XK_Escape, kEscape },
      { XK_F1, kF1 },
      { XK_F2, kF2 },
      { XK_F3, kF3 },
      { XK_F4, kF4 },
      { XK_F5, kF5 },
      { XK_F6, kF6 },
      { XK_F7, kF7 },
      { XK_F8, kF8 },
      { XK_F9, kF9 },
      { XK_F10, kF10 },
      { XK_F11, kF11 },
      { XK_F12, kF12 },
      { XK_Insert, kInsert },
      { XK_Delete, kDelete },
      { XK_Home, kHome },
      { XK_End, kEnd },
      { XK_Prior, kPageUp }, // XK_Page_Up
      { XK_Next, kPageDown }, // XK_Page_Down
      { XK_BackSpace, kBackspace },
      { XK_Up, kUp },
      { XK_Down, kDown },
      { XK_Left, kLeft },
      { XK_Right, kRight },
      { XK_Pause, kPause },
   };
   static const int nEntries = _countof(sym2code);
   for (int i = 0; i < nEntries; i++)
   {
      if (sym2code[i].sym == keysym)
      {
         if (pkeycode != NULL)
            *pkeycode = sym2code[i].code;
         return true;
      }
   }
   if (isprint(keysym))
   {
      if (pkeycode != NULL)
         *pkeycode = keysym;
      return true;
   }
   DebugMsg1("Unknown keysym %d\n", (int)keysym);
   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWindowX11
//

class cWindowX11 : public cComObject<IMPLEMENTSCP(IWindowX11, IWindowSink)>
{
public:
   cWindowX11(Display * pDisplay);

   virtual void OnFinalRelease();

   virtual tResult Create(int width, int height, int bpp, const char * pszTitle = NULL);

   virtual tResult GetWindowInfo(sWindowInfo * pInfo) const;

   virtual tResult SwapBuffers();

   virtual void DispatchEvent(const XEvent * pXEvent);

private:
   int m_bpp;
   Display * m_pDisplay;
   Window m_window;
   GLXContext m_glXContext;
};

///////////////////////////////////////

cWindowX11::cWindowX11(Display * pDisplay)
 : m_bpp(0),
   m_pDisplay(pDisplay),
   m_window(0),
   m_glXContext(NULL)
{
}

///////////////////////////////////////

void cWindowX11::OnFinalRelease()
{
   if (m_glXContext != NULL)
   {
      glXDestroyContext(m_pDisplay, m_glXContext);
      m_glXContext = NULL;
   }

   if (m_window != 0)
   {
      Verify(RevokeWindow(m_window) == this);

      XDestroyWindow(m_pDisplay, m_window);
      m_window = 0;
   }

   m_bpp = 0;
   m_pDisplay = NULL;
}

///////////////////////////////////////

tResult cWindowX11::Create(int width, int height, int bpp, const char * pszTitle /*= NULL*/)
{
   if (m_window != 0)
   {
      DebugMsg("WARNING: Window already created!\n");
      return E_FAIL;
   }

   if (width == 0 || height == 0)
   {
      DebugMsg("One or more invalid arguments\n");
      return E_FAIL;
   }

   if (m_pDisplay == NULL)
   {
      DebugMsg("No open Display on which to create the window\n");
      return E_FAIL;
   }

   if (!glXQueryExtension(m_pDisplay, NULL, NULL))
   {
      DebugMsg("Display doesn't support glx\n");
      return E_FAIL;
   }

   int attributes[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };

   XVisualInfo * pVi = glXChooseVisual(m_pDisplay, DefaultScreen(m_pDisplay), attributes);

   if (pVi == NULL)
   {
      DebugMsg("Could not get visual\n");
      return E_FAIL;
   }

   int screenWidth = DisplayWidth(m_pDisplay, pVi->screen);
   int screenHeight = DisplayHeight(m_pDisplay, pVi->screen);

   int x = (screenWidth - width) / 2;
   int y = (screenHeight - height) / 2;

   XSetWindowAttributes swa;
   swa.border_pixel = 0;
   swa.event_mask = ButtonPressMask | ButtonReleaseMask |
                    KeyPressMask | KeyReleaseMask |
                    PointerMotionMask | StructureNotifyMask;

   m_window = XCreateWindow(
      m_pDisplay, RootWindow(m_pDisplay, pVi->screen),
      x, y, width, height, 0, pVi->depth, InputOutput,
      pVi->visual, CWBorderPixel | CWEventMask, &swa);

   // @TODO: check for error

   if (pszTitle != NULL)
   {
      XStoreName(m_pDisplay, m_window, pszTitle);
   }

   XSizeHints sizeHints;
   sizeHints.flags = PPosition | PSize;
   sizeHints.x = x;
   sizeHints.y = y;
   sizeHints.width = width;
   sizeHints.height = height;
   XSetNormalHints(m_pDisplay, m_window, &sizeHints);

   m_glXContext = glXCreateContext(m_pDisplay, pVi, None, GL_TRUE);

   XFree(pVi);
   pVi = NULL;

   if (m_glXContext == NULL)
   {
      DebugMsg("Could not create glx context\n");
      return E_FAIL;
   }

   glXMakeCurrent(m_pDisplay, m_window, m_glXContext);

   XMapRaised(m_pDisplay, m_window);

   Verify(RegisterWindow(m_window, static_cast<IWindowX11 *>(this)));

   m_bpp = bpp;

   return S_OK;
}

///////////////////////////////////////

tResult cWindowX11::GetWindowInfo(sWindowInfo * pInfo) const
{
   Assert(pInfo != NULL);

   XWindowAttributes attr;
   XGetWindowAttributes(m_pDisplay, m_window, &attr);

   pInfo->width = attr.width;
   pInfo->height = attr.height;
   pInfo->bpp = m_bpp;
   pInfo->display = m_pDisplay;
   pInfo->window = m_window;

   return S_OK;
}

///////////////////////////////////////

tResult cWindowX11::SwapBuffers()
{
   glXSwapBuffers(m_pDisplay, m_window);
   return S_OK;
}

///////////////////////////////////////

void cWindowX11::DispatchEvent(const XEvent * pXEvent)
{
   typedef void (IWindowSink::*tOnKeyEvent)(long, bool, double);
   typedef void (IWindowSink::*tOnMouseEvent)(int x, int y, uint mouseState, double time);
   typedef void (IWindowSink::*tOnDestroy)(double time);
   typedef void (IWindowSink::*tOnResize)(int width, int height, double time);
   typedef void (IWindowSink::*tOnActivateApp)(bool bActive, double time);

   double eventTime = TimeGetSecs();

   switch (pXEvent->type)
   {
      case DestroyNotify:
      {
         ForEachConnection(&IWindowSink::OnDestroy, eventTime);
         break;
      }

      case ConfigureNotify:
      {
         ForEachConnection(&IWindowSink::OnResize, pXEvent->xconfigure.width, pXEvent->xconfigure.height, eventTime);
         break;
      }

      case KeyPress:
      case KeyRelease:
      {
         KeySym keysym = XLookupKeysym((XKeyEvent *)pXEvent, 0);
         long keycode;
         if (MapXKeysym(keysym, &keycode))
         {
            ForEachConnection(&IWindowSink::OnKeyEvent, keycode, pXEvent->type == KeyPress, eventTime);
         }
         break;
      }

      case ButtonPress:
      case ButtonRelease:
         // @TODO (dpalma 8-21-02): a less than ideal way to handle the mousewheel
         if (((XButtonEvent *)pXEvent)->button == 4)
         {
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelUp, true, eventTime);
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelUp, false, eventTime);
            break;
         }
         else if (((XButtonEvent *)pXEvent)->button == 5)
         {
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelDown, true, eventTime);
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelDown, false, eventTime);
            break;
         }
         // fall through
      case MotionNotify:
      {
         const XMotionEvent * pMotionEvent = reinterpret_cast<const XMotionEvent *>(pXEvent);

         uint state = pMotionEvent->state;

         uint mouseState = 0;
         if (state & Button1Mask)
            mouseState |= kLMouseDown;
         if (state & Button3Mask)
            mouseState |= kRMouseDown;
         if (state & Button2Mask)
            mouseState |= kMMouseDown;

         ForEachConnection(&IWindowSink::OnMouseEvent, pMotionEvent->x, pMotionEvent->y, mouseState, eventTime);
         break;
      }
   }
}

///////////////////////////////////////

IWindow * WindowCreate()
{
   if (g_pXDisplay == NULL)
   {
      g_pXDisplay = XOpenDisplay(NULL);
   }

   return static_cast<IWindow *>(new cWindowX11(g_pXDisplay));
}

IWindow * WindowCreate(int width, int height, int bpp, const char * pszTitle /*= NULL*/)
{
   IWindow * pWnd = WindowCreate();

   if (FAILED(pWnd->Create(width, height, bpp, pszTitle)))
   {
      SafeRelease(pWnd);
      return NULL;
   }

   return pWnd;
}

///////////////////////////////////////////////////////////////////////////////
