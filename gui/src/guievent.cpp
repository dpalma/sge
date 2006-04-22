///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guievent.h"
#include "guielementapi.h"

#include "keys.h"

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUIEvent);

///////////////////////////////////////////////////////////////////////////////

static struct sGUIEventCodeName
{
   tGUIEventCode code;
   const char * pszName;
}
g_guiEventNameTable[] =
{
   { kGUIEventNone, "noevent" },
   { kGUIEventFocus, "focus" },
   { kGUIEventBlur, "blur" },
   { kGUIEventDestroy, "destroy" },
   { kGUIEventMouseMove, "mousemove" },
   { kGUIEventMouseEnter, "mouseenter" },
   { kGUIEventMouseLeave, "mouseleave" },
   { kGUIEventMouseUp, "mouseup" },
   { kGUIEventMouseDown, "mousedown" },
   { kGUIEventMouseWheelUp, "mousewheelup" },
   { kGUIEventMouseWheelDown, "mousewheeldown" },
   { kGUIEventKeyUp, "keyup" },
   { kGUIEventKeyDown, "keydown" },
   { kGUIEventClick, "click" },
};

static bool g_guiEventNameTableSorted = false;

static int CDECL GUIEventCodeCompare(const void * elem1, const void * elem2)
{
   return ((*(tGUIEventCode *)elem1) - (*(tGUIEventCode *)elem2));
}

static void GUIEventNameTableSort()
{
   if (!g_guiEventNameTableSorted)
   {
      qsort(g_guiEventNameTable, _countof(g_guiEventNameTable), sizeof(g_guiEventNameTable[0]), GUIEventCodeCompare);
      g_guiEventNameTableSorted = true;
   }
}

tGUIString GUIEventName(tGUIEventCode code)
{
   GUIEventNameTableSort();

   sGUIEventCodeName * p = (sGUIEventCodeName *)bsearch(&code, g_guiEventNameTable,
      _countof(g_guiEventNameTable), sizeof(g_guiEventNameTable[0]), GUIEventCodeCompare);
   Assert(p != NULL);

   return tGUIString(p->pszName);
}

///////////////////////////////////////////////////////////////////////////////

static int CDECL GUIEventNameCompare(const void * elem1, const void * elem2)
{
   return _stricmp(((sGUIEventCodeName *)elem2)->pszName, ((sGUIEventCodeName *)elem1)->pszName);
}

tGUIEventCode GUIEventCode(const char * name)
{
   GUIEventNameTableSort();

   Assert(name != NULL);

   sGUIEventCodeName key;
   key.code = kGUIEventNone;
   key.pszName = name;

   sGUIEventCodeName * p = (sGUIEventCodeName *)bsearch(&key, g_guiEventNameTable,
      _countof(g_guiEventNameTable), sizeof(g_guiEventNameTable[0]), GUIEventNameCompare);

   if (p == NULL)
      return kGUIEventNone;

   return p->code;
}

///////////////////////////////////////////////////////////////////////////////

tGUIEventCode GUIEventCode(long key, bool down)
{
   switch (key)
   {
      case kMouseMove:
      {
         return kGUIEventMouseMove;
      }

      case kMouseLeft:
      case kMouseMiddle:
      case kMouseRight:
      {
         return down ? kGUIEventMouseDown : kGUIEventMouseUp;
      }

      case kMouseWheelUp:
      {
         return kGUIEventMouseWheelUp;
      }

      case kMouseWheelDown:
      {
         return kGUIEventMouseWheelDown;
      }

      default:
      {
         return down ? kGUIEventKeyDown : kGUIEventKeyUp;
      }
   }

   return kGUIEventNone;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEvent
//

///////////////////////////////////////

cGUIEvent::cGUIEvent()
 : m_eventCode(kGUIEventNone)
 , m_keyCode(-1)
 , m_bCancellable(false)
 , m_bCancelBubble(false)
 , m_bCtrlKeyDown(false)
 , m_bAltKeyDown(false)
 , m_bShiftKeyDown(false)
{
}

///////////////////////////////////////

cGUIEvent::cGUIEvent(tGUIEventCode eventCode, 
                     const tScreenPoint & mousePos, 
                     long keyCode, 
                     IGUIElement * pSource,
                     bool bCancellable,
                     bool bCtrlKeyDown,
                     bool bAltKeyDown,
                     bool bShiftKeyDown)
 : m_eventCode(eventCode)
 , m_mousePos(mousePos)
 , m_keyCode(keyCode)
 , m_pSource(CTAddRef(pSource))
 , m_bCancellable(bCancellable)
 , m_bCancelBubble(false)
 , m_bCtrlKeyDown(bCtrlKeyDown)
 , m_bAltKeyDown(bAltKeyDown)
 , m_bShiftKeyDown(bShiftKeyDown)
{
}

///////////////////////////////////////

tResult GUIEventCreate(tGUIEventCode eventCode,
                       tScreenPoint mousePos,
                       long keyCode,
                       IGUIElement * pSource,
                       bool bCancellable,
                       bool bCtrlKeyDown,
                       bool bAltKeyDown,
                       bool bShiftKeyDown,
                       IGUIEvent * * ppEvent)
{
   if (ppEvent == NULL)
   {
      return E_POINTER;
   }
   cGUIEvent * pGUIEvent = new cGUIEvent(eventCode, mousePos, keyCode,
      pSource, bCancellable, bCtrlKeyDown, bAltKeyDown, bShiftKeyDown);
   if (pGUIEvent == NULL)
   {
      return E_OUTOFMEMORY;
   }
   *ppEvent = static_cast<IGUIEvent*>(pGUIEvent);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIEvent::GetEventCode(tGUIEventCode * pEventCode)
{
   if (pEventCode == NULL)
      return E_POINTER;
   *pEventCode = m_eventCode;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIEvent::GetMousePosition(tScreenPoint * pMousePos)
{
   if (pMousePos == NULL)
   {
      return E_POINTER;
   }
   *pMousePos = m_mousePos;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIEvent::GetKeyCode(long * pKeyCode)
{
   if (pKeyCode == NULL)
   {
      return E_POINTER;
   }
   *pKeyCode = m_keyCode;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIEvent::GetSourceElement(IGUIElement * * ppElement)
{
   return m_pSource.GetPointer(ppElement);
}

///////////////////////////////////////

bool cGUIEvent::IsCancellable() const
{
   return m_bCancellable;
}

///////////////////////////////////////

tResult cGUIEvent::GetCancelBubble()
{
   return m_bCancelBubble ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cGUIEvent::SetCancelBubble(bool bCancel)
{
   if (bCancel && !IsCancellable())
   {
      return E_FAIL;
   }
   m_bCancelBubble = bCancel;
   return S_OK;
}

///////////////////////////////////////

bool cGUIEvent::IsCtrlKeyDown() const
{
   return m_bCtrlKeyDown;
}

///////////////////////////////////////

bool cGUIEvent::IsAltKeyDown() const
{
   return m_bAltKeyDown;
}

///////////////////////////////////////

bool cGUIEvent::IsShiftKeyDown() const
{
   return m_bShiftKeyDown;
}

///////////////////////////////////////////////////////////////////////////////
