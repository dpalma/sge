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

LOG_DEFINE_ENABLE_CHANNEL(Window, true);

#define LocalMsg(msg)            DebugMsgEx(Window,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(Window,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(Window,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(Window,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(Window,(msg),(a),(b),(c),(d))

///////////////////////////////////////////////////////////////////////////////

static int CompareKeySym(const void * elem1, const void * elem2)
{
   return (*(long*)elem1) - (*(long*)elem2);
}

static bool MapXKeysym(KeySym keysym, long * pKeyCode)
{
   static struct
   {
      long keySym;
      long keyCode;
   }
   keyCodeTable[] =
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

   static bool keyCodeTableSorted = false;

   if (!keyCodeTableSorted)
   {
      qsort(keyCodeTable, _countof(keyCodeTable), sizeof(keyCodeTable[0]), CompareKeySym);
      keyCodeTableSorted = true;
   }

#if 0
   // TODO: use bsearch
#else
   for (int i = 0; i < _countof(keyCodeTable); i++)
   {
      if (keyCodeTable[i].keySym == keysym)
      {
         if (pKeyCode != NULL)
         {
            *pKeyCode = keyCodeTable[i].keyCode;
         }
         return true;
      }
   }
#endif

   if (isprint(keysym))
   {
      if (pKeyCode != NULL)
      {
         *pKeyCode = keysym;
      }
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
   friend IWindowX11 * FindWindow(Window window);
   friend tResult GetDisplay(Display * * ppDisplay);

   cWindowX11();

   virtual void OnFinalRelease();

   virtual tResult Create(int width, int height, int bpp, const char * pszTitle = NULL);
   virtual tResult GetWindowInfo(sWindowInfo * pInfo) const;
   virtual tResult SwapBuffers();

   virtual void DispatchEvent(const XEvent * pXEvent);

private:
   static int HandleXError(Display * display, XErrorEvent * event);
   static XErrorHandler gm_nextErrorHandler;

   typedef std::map<Window, IWindowX11 *> tWindowMap;
   static tWindowMap gm_windowMap;

   static Display * gm_display;

   int m_bpp;
   Window m_window;
   GLXContext m_context;
};

///////////////////////////////////////

XErrorHandler cWindowX11::gm_nextErrorHandler = NULL;

///////////////////////////////////////

cWindowX11::tWindowMap cWindowX11::gm_windowMap;

///////////////////////////////////////

Display * cWindowX11::gm_display = NULL;

///////////////////////////////////////

cWindowX11::cWindowX11()
 : m_bpp(0),
   m_window(0),
   m_context(NULL)
{
}

///////////////////////////////////////

void cWindowX11::OnFinalRelease()
{
   m_bpp = 0;

   if (m_context != NULL)
   {
      glXDestroyContext(gm_display, m_context);
      m_context = NULL;
   }

   if (m_window != 0)
   {
      gm_windowMap.erase(m_window);

      XDestroyWindow(gm_display, m_window);
      m_window = 0;
   }

   if (gm_windowMap.empty())
   {
      if (gm_nextErrorHandler != NULL)
      {
         XSetErrorHandler(gm_nextErrorHandler);
         gm_nextErrorHandler = NULL;
      }

      XCloseDisplay(gm_display);
      gm_display = NULL;
   }
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

   if (gm_display == NULL)
   {
      gm_display = XOpenDisplay(NULL);

      Assert(gm_nextErrorHandler == NULL);
      gm_nextErrorHandler = XSetErrorHandler(HandleXError);
   }

   if (gm_display == NULL)
   {
      DebugMsg("Unable to open X display\n");
      return E_FAIL;
   }

   if (!glXQueryExtension(gm_display, NULL, NULL))
   {
      DebugMsg("Display doesn't support glx\n");
      return E_FAIL;
   }

   int attributes[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };

   XVisualInfo * pVi = glXChooseVisual(gm_display, DefaultScreen(gm_display), attributes);

   if (pVi == NULL)
   {
      DebugMsg("Could not get visual\n");
      return E_FAIL;
   }

   int screenWidth = DisplayWidth(gm_display, pVi->screen);
   int screenHeight = DisplayHeight(gm_display, pVi->screen);

   LocalMsg3("Screen %d is %d x %d\n", pVi->screen, screenWidth, screenHeight);
   LocalMsg3("Using visual %x, depth %d, class %d\n", pVi->visualid, pVi->depth, pVi->c_class);

   Colormap colorMap = XCreateColormap(gm_display, RootWindow(gm_display, pVi->screen),
      pVi->visual, AllocNone);

   XSetWindowAttributes swa;
   swa.border_pixel = 0;
   swa.colormap = colorMap;
   swa.event_mask = ButtonPressMask | ButtonReleaseMask |
                    KeyPressMask | KeyReleaseMask |
                    PointerMotionMask | StructureNotifyMask;

   int x = (screenWidth - width) / 2;
   int y = (screenHeight - height) / 2;

   m_window = XCreateWindow(
      gm_display, RootWindow(gm_display, pVi->screen),
      x, y, width, height, 0, pVi->depth, InputOutput,
      pVi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);

   std::pair<tWindowMap::iterator, bool> result = gm_windowMap.insert(std::make_pair(m_window, this));
   Assert(result.second);

   if (pszTitle != NULL)
   {
      XStoreName(gm_display, m_window, pszTitle);
   }

   XSizeHints sizeHints;
   sizeHints.flags = PPosition | PSize;
   sizeHints.x = x;
   sizeHints.y = y;
   sizeHints.width = width;
   sizeHints.height = height;
   XSetNormalHints(gm_display, m_window, &sizeHints);

   m_context = glXCreateContext(gm_display, pVi, None, GL_TRUE);

   XFree(pVi);
   pVi = NULL;

   if (m_context == NULL)
   {
      DebugMsg("Could not create glx context\n");
      return E_FAIL;
   }

   glXMakeCurrent(gm_display, m_window, m_context);

   XMapRaised(gm_display, m_window);

   m_bpp = bpp;

   return S_OK;
}

///////////////////////////////////////

tResult cWindowX11::GetWindowInfo(sWindowInfo * pInfo) const
{
   Assert(pInfo != NULL);

   XWindowAttributes attr;
   XGetWindowAttributes(gm_display, m_window, &attr);

   pInfo->width = attr.width;
   pInfo->height = attr.height;
   pInfo->bpp = m_bpp;
   pInfo->display = gm_display;
   pInfo->window = m_window;

   return S_OK;
}

///////////////////////////////////////

tResult cWindowX11::SwapBuffers()
{
   glXSwapBuffers(gm_display, m_window);
   return S_OK;
}

///////////////////////////////////////

void cWindowX11::DispatchEvent(const XEvent * pXEvent)
{
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
         if (pXEvent->xbutton.button == 4)
         {
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelUp, true, eventTime);
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelUp, false, eventTime);
            break;
         }
         else if (pXEvent->xbutton.button == 5)
         {
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelDown, true, eventTime);
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)kMouseWheelDown, false, eventTime);
            break;
         }
         // fall through
      case MotionNotify:
      {
         uint state = pXEvent->xmotion.state;

         uint mouseState = 0;
         if (state & Button1Mask)
            mouseState |= kLMouseDown;
         if (state & Button3Mask)
            mouseState |= kRMouseDown;
         if (state & Button2Mask)
            mouseState |= kMMouseDown;

         ForEachConnection(&IWindowSink::OnMouseEvent, pXEvent->xmotion.x, pXEvent->xmotion.y, mouseState, eventTime);
         break;
      }
   }
}

///////////////////////////////////////

int cWindowX11::HandleXError(Display * display, XErrorEvent * event)
{
   LocalMsg4("X error %d, %d, %d, %d\n", event->serial, event->error_code, event->request_code, event->minor_code);
   Assert(gm_nextErrorHandler != NULL);
   return gm_nextErrorHandler(display, event);
}

///////////////////////////////////////

IWindowX11 * FindWindow(Window window)
{
   cWindowX11::tWindowMap::iterator iter = cWindowX11::gm_windowMap.find(window);
   if (iter != cWindowX11::gm_windowMap.end())
   {
      return CTAddRef(iter->second);
   }
   return NULL;
}

///////////////////////////////////////

tResult GetDisplay(Display * * ppDisplay)
{
   Assert(ppDisplay != NULL);
   if (ppDisplay == NULL)
   {
      return E_POINTER;
   }
   if (cWindowX11::gm_display == NULL)
   {
      return S_FALSE;
   }
   *ppDisplay = cWindowX11::gm_display;
   return S_OK;
}

///////////////////////////////////////

IWindow * WindowCreate()
{
   return static_cast<IWindow *>(new cWindowX11);
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
