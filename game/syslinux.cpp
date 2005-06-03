///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sys.h"

#include "inputapi.h"

#include "globalobj.h"
#include "keys.h"
#include "matrix4.h"
#include "techtime.h"

#include <locale>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(XEvents);

bool              g_bExiting = false;
int               g_mousex = 0, g_mousey = 0;

Display *         g_display = NULL;
Window            g_window = 0;
GLXContext        g_context = NULL;
XErrorHandler     g_nextErrorHandler = NULL;

///////////////////////////////////////////////////////////////////////////////

bool IsExiting()
{
   return g_bExiting;
}

///////////////////////////////////////////////////////////////////////////////

void SysAppActivate(bool active)
{
}

///////////////////////////////////////////////////////////////////////////////

void SysQuit()
{
   g_bExiting = true;
}

///////////////////////////////////////////////////////////////////////////////

void SysGetMousePos(int * px, int * py)
{
   *px = g_mousex;
   *py = g_mousey;
}

///////////////////////////////////////////////////////////////////////////////

bool SysGetClipboardString(char * psz, int max)
{
   return false;
}

///////////////////////////////////////////////////////////////////////////////

bool SysSetClipboardString(const char * psz)
{
   return false;
}

///////////////////////////////////////////////////////////////////////////////

static int SysHandleXError(Display * display, XErrorEvent * event)
{
   ErrorMsg4("X error %d, %d, %d, %d\n", event->serial, event->error_code, event->request_code, event->minor_code);
   Assert(g_nextErrorHandler != NULL);
   return g_nextErrorHandler(display, event);
}

///////////////////////////////////////////////////////////////////////////////

HANDLE SysCreateWindow(const tChar * pszTitle, int width, int height)
{
   if (g_window == 0)
   {
      if (g_display == NULL)
      {
         g_display = XOpenDisplay(NULL);
         if (g_display == NULL)
         {
            ErrorMsg("Unable to open X display connection\n");
            return NULL;
         }

         Assert(g_nextErrorHandler == NULL);
         g_nextErrorHandler = XSetErrorHandler(SysHandleXError);
      }

      if (!glXQueryExtension(g_display, NULL, NULL))
      {
         ErrorMsg("X display doesn't support glx\n");
         return NULL;
      }

      int attributes[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };
      XVisualInfo * pVi = glXChooseVisual(g_display, DefaultScreen(g_display), attributes);
      if (pVi == NULL)
      {
         ErrorMsg("Could not get glX visual\n");
         return NULL;
      }

      int screenWidth = DisplayWidth(g_display, pVi->screen);
      int screenHeight = DisplayHeight(g_display, pVi->screen);

      DebugMsg3("Screen %d is %d x %d\n", pVi->screen, screenWidth, screenHeight);
      DebugMsg3("Using visual %x, depth %d, class %d\n", pVi->visualid, pVi->depth, pVi->c_class);

      Colormap colorMap = XCreateColormap(g_display, RootWindow(g_display, pVi->screen),
         pVi->visual, AllocNone);

      XSetWindowAttributes swa;
      swa.border_pixel = 0;
      swa.colormap = colorMap;
      swa.event_mask = ButtonPressMask | ButtonReleaseMask |
                       KeyPressMask | KeyReleaseMask |
                       PointerMotionMask | StructureNotifyMask;

      int x = (screenWidth - width) / 2;
      int y = (screenHeight - height) / 2;

      g_window = XCreateWindow(
         g_display, RootWindow(g_display, pVi->screen),
         x, y, width, height, 0, pVi->depth, InputOutput,
         pVi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);

      if ((pszTitle != NULL) && (strlen(pszTitle) > 0))
      {
         XStoreName(g_display, g_window, pszTitle);
      }

      XSizeHints sizeHints;
      sizeHints.flags = PPosition | PSize;
      sizeHints.x = x;
      sizeHints.y = y;
      sizeHints.width = width;
      sizeHints.height = height;
      XSetNormalHints(g_display, g_window, &sizeHints);

      g_context = glXCreateContext(g_display, pVi, None, GL_TRUE);

      XFree(pVi);
      pVi = NULL;

      if (g_context == NULL)
      {
         ErrorMsg("Could not create glX context\n");
         return NULL;
      }

      glXMakeCurrent(g_display, g_window, g_context);

      XMapRaised(g_display, g_window);
   }

   return reinterpret_cast<HANDLE>(g_window);
}

///////////////////////////////////////////////////////////////////////////////

void SysSwapBuffers()
{
   glXSwapBuffers(g_display, g_window);
}

///////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
static const char * XEventName(int event)
{
   switch (event)
   {
#define CASE(e) case e: return #e
      CASE(KeyPress);
      CASE(KeyRelease);
      CASE(ButtonPress);
      CASE(ButtonRelease);
      CASE(MotionNotify);
      CASE(EnterNotify);
      CASE(LeaveNotify);
      CASE(FocusIn);
      CASE(FocusOut);
      CASE(KeymapNotify);
      CASE(Expose);
      CASE(GraphicsExpose);
      CASE(NoExpose);
      CASE(VisibilityNotify);
      CASE(CreateNotify);
      CASE(DestroyNotify);
      CASE(UnmapNotify);
      CASE(MapNotify);
      CASE(MapRequest);
      CASE(ReparentNotify);
      CASE(ConfigureNotify);
      CASE(ConfigureRequest);
      CASE(GravityNotify);
      CASE(ResizeRequest);
      CASE(CirculateNotify);
      CASE(CirculateRequest);
      CASE(PropertyNotify);
      CASE(SelectionClear);
      CASE(SelectionRequest);
      CASE(SelectionNotify);
      CASE(ColormapNotify);
      CASE(ClientMessage);
      CASE(MappingNotify);
#undef CASE
      default: return "unknown";
   }
}
#endif

///////////////////////////////////////////////////////////////////////////////

static int SysCompareKeySym(const void * elem1, const void * elem2)
{
   return (*(long*)elem1) - (*(long*)elem2);
}

static bool SysMapXKeysym(KeySym keysym, long * pKeyCode)
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
      qsort(keyCodeTable, _countof(keyCodeTable), sizeof(keyCodeTable[0]), SysCompareKeySym);
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

   ErrorMsg1("Unknown keysym %d\n", static_cast<int>(keysym));
   return false;
}

///////////////////////////////////////////////////////////////////////////////

extern void ResizeHack(int width, int height);

int SysEventLoop(void (* pfnFrameHandler)())
{
   if (!g_display || !g_window)
   {
      // Window should have been created during MainInit().
      return EXIT_FAILURE;
   }

   UseGlobal(Input);

   while (!g_bExiting)
   {
      XFlush(g_display);
      if (XPending(g_display))
      {
         XEvent event;
         XNextEvent(g_display, &event);

         double eventTime = TimeGetSecs();

         DebugMsgEx2(XEvents, "Event %s at %f\n", XEventName(event.type), eventTime);

         if (event.type == MotionNotify)
         {
            g_mousex = event.xmotion.x;
            g_mousey = event.xmotion.y;
         }

         switch (event.type)
         {
            case DestroyNotify:
            {
               DebugMsg1("Window destroyed at %.3f\n", eventTime);
               break;
            }

            case ConfigureNotify:
            {
               DebugMsg1("Window configured at %.3f\n", eventTime);
               ResizeHack(event.xconfigure.width, event.xconfigure.height);
               break;
            }

            case KeyPress:
            case KeyRelease:
            {
               KeySym keysym = XLookupKeysym((XKeyEvent*)&event, 0);
               long keycode;
               if (SysMapXKeysym(keysym, &keycode))
               {
                  pInput->ReportKeyEvent(keycode, event.type == KeyPress, eventTime);
               }
               break;
            }

            case ButtonPress:
            case ButtonRelease:
            {
               if (event.xbutton.button == 4)
               {
                  pInput->ReportKeyEvent(kMouseWheelUp, true, eventTime);
                  pInput->ReportKeyEvent(kMouseWheelUp, false, eventTime);
                  break;
               }
               else if (event.xbutton.button == 5)
               {
                  pInput->ReportKeyEvent(kMouseWheelDown, true, eventTime);
                  pInput->ReportKeyEvent(kMouseWheelDown, false, eventTime);
                  break;
               }
               // fall through
            }
            case MotionNotify:
            {
               uint mouseState = 0;
               uint state = event.xmotion.state;
               if (state & Button1Mask)
                  mouseState |= kLMouseDown;
               if (state & Button3Mask)
                  mouseState |= kRMouseDown;
               if (state & Button2Mask)
                  mouseState |= kMMouseDown;
               pInput->ReportMouseEvent(event.xmotion.x, event.xmotion.y, mouseState, eventTime);
               break;
            }
         }
      }
      else
      {
         (*pfnFrameHandler)();
      }
   }

   return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
