///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "uievent.h"
#include "ui.h"
#include "keys.h"

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(UIEvent);

///////////////////////////////////////////////////////////////////////////////

static struct sEventCodeName
{
   eUIEventCode code;
   const char * pszName;
}
g_uiEventNameTable[] =
{
   { kEventFocus, "focus" },
   { kEventBlur, "blur" },
   { kEventDestroy, "destroy" },
   { kEventMouseMove, "mousemove" },
   { kEventMouseEnter, "mouseenter" },
   { kEventMouseLeave, "mouseleave" },
   { kEventMouseUp, "mouseup" },
   { kEventMouseDown, "mousedown" },
   { kEventMouseWheelUp, "mousewheelup" },
   { kEventMouseWheelDown, "mousewheeldown" },
   { kEventKeyUp, "keyup" },
   { kEventKeyDown, "keydown" },
   { kEventClick, "click" },
};

static bool g_uiEventNameTableSorted = false;

///////////////////////////////////////////////////////////////////////////////

static int CDECL CompareEventCode(const void * elem1, const void * elem2)
{
   if ((*(eUIEventCode *)elem1) < (*(eUIEventCode *)elem2))
      return -1;
   else if ((*(eUIEventCode *)elem1) > (*(eUIEventCode *)elem2))
      return 1;
   else
      return 0;
}

static void EventNameTableSort()
{
   if (!g_uiEventNameTableSorted)
   {
      qsort(g_uiEventNameTable, _countof(g_uiEventNameTable), sizeof(g_uiEventNameTable[0]), CompareEventCode);
      g_uiEventNameTableSorted = true;
   }
}

cUIString UIEventName(eUIEventCode code)
{
   EventNameTableSort();

   sEventCodeName * p = (sEventCodeName *)bsearch(&code, g_uiEventNameTable,
      _countof(g_uiEventNameTable), sizeof(g_uiEventNameTable[0]), CompareEventCode);
   Assert(p != NULL);

   return cUIString(p->pszName);
}

///////////////////////////////////////////////////////////////////////////////

static int CDECL CompareEventName(const void * elem1, const void * elem2)
{
   return stricmp(((sEventCodeName *)elem2)->pszName, ((sEventCodeName *)elem1)->pszName);
}

eUIEventCode UIEventCode(const char * name)
{
   EventNameTableSort();

   Assert(name != NULL);

   sEventCodeName key;
   key.code = kEventERROR;
   key.pszName = name;

   sEventCodeName * p = (sEventCodeName *)bsearch(&key, g_uiEventNameTable,
      _countof(g_uiEventNameTable), sizeof(g_uiEventNameTable[0]), CompareEventName);

   if (p == NULL)
      return kEventERROR;

   return p->code;
}

///////////////////////////////////////////////////////////////////////////////

eUIEventCode UIEventCode(long key, bool down)
{
   switch (key)
   {
      case kMouseMove:
      {
         return kEventMouseMove;
      }

      case kMouseLeft:
      case kMouseMiddle:
      case kMouseRight:
      {
         return down ? kEventMouseDown : kEventMouseUp;
      }

      case kMouseWheelUp:
      {
         return kEventMouseWheelUp;
      }

      case kMouseWheelDown:
      {
         return kEventMouseWheelDown;
      }

      default:
      {
         return down ? kEventKeyDown : kEventKeyUp;
      }
   }

   return kEventERROR;
}

///////////////////////////////////////////////////////////////////////////////

bool UIDispatchEvent(cUIComponent * pTarget, const cUIEvent * pEvent, tUIResult * pResult)
{
   Assert(pTarget != NULL && pEvent != NULL && pResult != NULL);
   DebugMsgIfEx2(UIEvent, pEvent->code != kEventMouseMove,
      "Dispatching event %s to %x\n", UIEventName(pEvent->code).c_str(), pTarget);
   return pTarget->OnEvent(pEvent, pResult);
}

///////////////////////////////////////////////////////////////////////////////

bool UIBubbleEvent(cUIComponent * pStart, const cUIEvent * pEvent, tUIResult * pResult)
{
   Assert(pStart != NULL && pEvent != NULL && pResult != NULL);
   cUIComponent * pTarget = pStart;
   while (pTarget != NULL)
   {
      if (UIDispatchEvent(pTarget, pEvent, pResult))
         return true;
      pTarget = pTarget->GetParent();
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIEvent
//

///////////////////////////////////////

cUIEvent::cUIEvent()
 : code(kEventERROR),
   pSrc(NULL),
   mousePos(-1,-1),
   keyCode(-1)
{
}

///////////////////////////////////////

cUIEvent::cUIEvent(const cUIEvent & other)
{
   operator=(other);
}

///////////////////////////////////////

const cUIEvent & cUIEvent::operator=(const cUIEvent & other)
{
   code = other.code;
   pSrc = other.pSrc;
   mousePos = other.mousePos;
   keyCode = other.keyCode;
   return *this;
}

///////////////////////////////////////////////////////////////////////////////
