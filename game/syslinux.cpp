///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "gcommon.h"
#include "matrix4.h"
#include "window.h"
#include "techtime.h"

#include <locale>
#include <X11/Xlib.h>
#include <X11/keysym.h>

LOG_DEFINE_CHANNEL(XEvents);

bool g_bExiting = false;
long g_mousex, g_mousey;

Display * g_display = NULL;
Window g_window = NULL;
GLXContext g_context = NULL;

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

HANDLE SysGetInstanceHandle()
{
   return NULL;
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

HANDLE SysCreateWindow(const tChar * pszTitle, int width, int height)
{
}

///////////////////////////////////////////////////////////////////////////////

void SysSwapBuffers()
{
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

int main(int argc, char * argv[])
{
   if (!MainInit(argc, argv))
   {
      MainTerm();
      return EXIT_FAILURE;
   }

   Display * display = NULL;
   if (GetDisplay(&display) != S_OK)
   {
      DebugMsg("Unable to get X display\n");
      return EXIT_FAILURE;
   }

   for (;;)
   {
      XFlush(display);
      if (XPending(display))
      {
         XEvent event;
         XNextEvent(display, &event);

         DebugMsgEx2(XEvents, "Event %s at %f\n", XEventName(event.type), TimeGetSecs());

         if (event.type == MotionNotify)
         {
            g_mousex = event.xmotion.x;
            g_mousey = event.xmotion.y;
         }

         cAutoIPtr<IWindowX11> pWindowX11 = FindWindow(event.xany.window);

         if (pWindowX11 != NULL)
         {
            pWindowX11->DispatchEvent(&event);
         }

	      if (g_bExiting)
         {
            break;
         }
      }
      else
      {
         MainFrame();
      }
   }

   MainTerm();

   return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
