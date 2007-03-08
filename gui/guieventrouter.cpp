///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

LOG_DEFINE_CHANNEL(GUIEventRouter);

#ifdef HAVE_UNITTESTPP

#include "guieventrouter.h"
#include "guieventroutertem.h"

#include "guicontainerbase.h"
#include "guicontainerbasetem.h"

#include "platform/keys.h"
#include "platform/inputapi.h"

#include "tech/connptimpl.h"

#include "UnitTest++.h"

#include "tech/dbgalloc.h" // must be last header


typedef std::list<IGUIEventListener *> tGUIEventListenerList;


///////////////////////////////////////////////////////////////////////////////

static eGUIEventCode GUIEventCode(IGUIEvent * pEvent)
{
   eGUIEventCode eventCode = kGUIEventNone;
   if ((pEvent != NULL) && (pEvent->GetEventCode(&eventCode) == S_OK))
   {
      return eventCode;
   }
   else
   {
      return kGUIEventNone;
   }
}


///////////////////////////////////////////////////////////////////////////////

static bool GUIEventMousePositionIs(IGUIEvent * pEvent, int x, int y)
{
   tScreenPoint mousePos;
   if ((pEvent != NULL) && (pEvent->GetMousePosition(&mousePos) == S_OK))
   {
      return (mousePos.x == x) && (mousePos.y == y);
   }
   else
   {
      return false;
   }
}


///////////////////////////////////////////////////////////////////////////////

static bool GUIEventSourceElementIs(IGUIEvent * pEvent, IGUIElement * pElement)
{
   cAutoIPtr<IGUIElement> pEventElement;
   if ((pEvent != NULL) && (pEvent->GetSourceElement(&pEventElement) == S_OK))
   {
      return CTIsSameObject(pEventElement, pElement);
   }
   else
   {
      return false;
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITestElement
//

class cGUITestElement : public cComObject<cGUIElementBase<IGUIElement>, &IID_IGUIElement>
{
public:
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEventCollector
//

class cGUIEventCollector : public cComObject<IMPLEMENTS(IGUIEventListener)>
{
public:
   cGUIEventCollector();
   ~cGUIEventCollector();
   virtual tResult OnEvent(IGUIEvent * pEvent);
   uint GetEventCount() const { return m_events.size(); }
   IGUIEvent * AccessEvent(uint index) { return (index < m_events.size()) ? m_events[index] : NULL; }
   std::vector<IGUIEvent *>::const_iterator GetFirstEvent() const { return m_events.begin(); }
   std::vector<IGUIEvent *>::const_iterator GetLastEvent() const { return m_events.end(); }
private:
   typedef std::vector<IGUIEvent *> tGUIEventVector;
   tGUIEventVector m_events;
};

////////////////////////////////////////

cGUIEventCollector::cGUIEventCollector()
{
}

////////////////////////////////////////

cGUIEventCollector::~cGUIEventCollector()
{
   std::for_each(m_events.begin(), m_events.end(), CTInterfaceMethod(&IGUIEvent::Release));
   m_events.clear();
}

////////////////////////////////////////

tResult cGUIEventCollector::OnEvent(IGUIEvent * pEvent)
{
   if (pEvent != NULL)
   {
      m_events.push_back(CTAddRef(pEvent));
   }
   return S_OK;
}

////////////////////////////////////////

class cStackGUIEventCollector : public cGUIEventCollector
{
protected:
   virtual void DeleteThis() {}
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEventRouterFixture
//

class cGUIEventRouterFixture : public cComObject<cGUIEventRouter<cGUIEventRouterFixture,
                                                                 cGUIContainerBase<IGUIContainerElement>,
                                                                 tGUIEventListenerList::iterator>,
                                                 &IID_IGUIContainerElement>
{
   // Prevent heap-based objects
#ifdef DBGALLOC_MAPPED
#undef new
#endif
   static void * operator new(size_t);
#ifdef DBGALLOC_MAPPED
#define new DebugNew
#endif
   static void operator delete(void *) {}

public:
   cGUIEventRouterFixture();
   ~cGUIEventRouterFixture();

   virtual void DeleteThis() {}

   tResult GetHitElement(const tScreenPoint & point, IGUIElement * * ppElement) const;

   // IGUIEventRouter methods
   virtual tResult AddEventListener(IGUIEventListener * pListener);
   virtual tResult RemoveEventListener(IGUIEventListener * pListener);

   tResult CreateTestElement(const tChar * pszId, const tGUIPoint & position, const tGUISize & size, IGUIElement * * ppElement);

   inline tResult CreateTestElement(const tChar * pszId, const tGUIPoint & position, const tGUISize & size)
   {
      return CreateTestElement(pszId, position, size, NULL);
   }

   struct sHitTest
   {
      void operator()(IGUIElement * pElement)
      {
         const tGUIPoint & position = pElement->GetPosition();
         const tGUISize & size = pElement->GetSize();

         tRectf rect(position.x, position.y, position.x + size.width, position.y + size.height);

         if (rect.PtInside(static_cast<float>(m_point.x), static_cast<float>(m_point.y)))
         {
            m_pHit = pElement;
         }
      }

      tScreenPoint m_point;
      IGUIElement * m_pHit;
   };

   tGUIEventListenerList::iterator BeginEventListeners() { return m_eventListeners.begin(); }
   tGUIEventListenerList::iterator EndEventListeners() { return m_eventListeners.end(); }

   cStackGUIEventCollector m_eventCollector;

private:
   tGUIEventListenerList m_eventListeners;
};

////////////////////////////////////////

cGUIEventRouterFixture::cGUIEventRouterFixture()
{
   Verify(AddEventListener(static_cast<IGUIEventListener*>(&m_eventCollector)) == S_OK);
}

////////////////////////////////////////

cGUIEventRouterFixture::~cGUIEventRouterFixture()
{
   Verify(RemoveEventListener(static_cast<IGUIEventListener*>(&m_eventCollector)) == S_OK);

   std::for_each(m_eventListeners.begin(), m_eventListeners.end(), CTInterfaceMethod(&IGUIEventListener::Release));
   m_eventListeners.clear();
}

////////////////////////////////////////

tResult cGUIEventRouterFixture::GetHitElement(const tScreenPoint & point, IGUIElement * * ppElement) const
{
   sHitTest ht;
   ht.m_point = point;
   ht.m_pHit = NULL;
   ht = ForEachElement(ht);
   if (ht.m_pHit == NULL)
   {
      return S_FALSE;
   }
   *ppElement = CTAddRef(ht.m_pHit);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIEventRouterFixture::AddEventListener(IGUIEventListener * pListener)
{
   return add_interface(m_eventListeners, pListener) ? S_OK : E_FAIL;
}

///////////////////////////////////////

tResult cGUIEventRouterFixture::RemoveEventListener(IGUIEventListener * pListener)
{
   return remove_interface(m_eventListeners, pListener) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cGUIEventRouterFixture::CreateTestElement(const tChar * pszId,
                                                  const tGUIPoint & position,
                                                  const tGUISize & size,
                                                  IGUIElement * * ppElement)
{
   cAutoIPtr<IGUIElement> pElement(static_cast<IGUIElement*>(new cGUITestElement));
   if (!pElement)
   {
      return E_OUTOFMEMORY;
   }
   if (pszId != NULL)
   {
      pElement->SetId(pszId);
   }
   pElement->SetPosition(position);
   pElement->SetSize(size);
   tResult result = AddElement(pElement);
   if (result == S_OK && ppElement != NULL)
   {
      *ppElement = CTAddRef(pElement);
   }
   return result;
}

///////////////////////////////////////////////////////////////////////////////
// Move the mouse inside an element and check that a mouse enter and 
// mouse move event occurred, and that the mouse enter happens before
// the mouse move.

TEST_FIXTURE(cGUIEventRouterFixture, MouseEnter)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("mouseEnterElement"), tGUIPoint(1,1), tGUISize(1,1)));

   static const sInputEvent inputEvents[] =
   {
      { kMouseMove, kMK_None, false, cVec2<int>(0,0), .01 }, // outside
      { kMouseMove, kMK_None, false, cVec2<int>(1,1), .02 }, // inside
   };

   for (int i = 0; i < _countof(inputEvents); ++i)
   {
      HandleInputEvent(&inputEvents[i]);
   }

   CHECK_EQUAL(2, m_eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseEnter, GUIEventCode(m_eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventMouseMove, GUIEventCode(m_eventCollector.AccessEvent(1)));
}

///////////////////////////////////////////////////////////////////////////////
// Move the mouse inside then back outside of an element and check
// that a mouse enter and leave event were dispatched.

TEST_FIXTURE(cGUIEventRouterFixture, MouseEnterLeave)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("mouseEnterLeaveElement"), tGUIPoint(1,1), tGUISize(1,1)));

   static const sInputEvent inputEvents[] =
   {
      { kMouseMove, kMK_None, false, cVec2<int>(0,0), .01 }, // outside
      { kMouseMove, kMK_None, false, cVec2<int>(1,1), .02 }, // inside
      { kMouseMove, kMK_None, false, cVec2<int>(0,0), .03 }, // outside
   };

   for (int i = 0; i < _countof(inputEvents); ++i)
   {
      HandleInputEvent(&inputEvents[i]);
   }

   CHECK_EQUAL(3, m_eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseEnter, GUIEventCode(m_eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventMouseMove, GUIEventCode(m_eventCollector.AccessEvent(1)));
   CHECK_EQUAL(kGUIEventMouseLeave, GUIEventCode(m_eventCollector.AccessEvent(2)));
}

///////////////////////////////////////////////////////////////////////////////
// Generate mouse wheel events just like the system does (each event happens
// twice--once with the key-pressed flag true and again with it false);
// Check that mouse wheel events go to the element the mouse is over and that
// only one GUI event occurs for the two system mouse wheel events

TEST_FIXTURE(cGUIEventRouterFixture, MouseWheel)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("mouseWheelElement"), tGUIPoint(1,1), tGUISize(1,1)));

   static const sInputEvent inputEvents[] =
   {
      { kMouseWheelDown, kMK_None, true, cVec2<int>(0,0), .01 }, // outside
      { kMouseWheelDown, kMK_None, false, cVec2<int>(0,0), .02 }, // outside
      { kMouseWheelUp, kMK_None, true, cVec2<int>(1,1), .03 }, // inside
      { kMouseWheelUp, kMK_None, false, cVec2<int>(1,1), .04 }, // inside
   };

   // TODO: Should mouse wheel events go to the focussed element?

   for (int i = 0; i < _countof(inputEvents); ++i)
   {
      HandleInputEvent(&inputEvents[i]);
   }

   CHECK_EQUAL(1, m_eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseWheelUp, GUIEventCode(m_eventCollector.AccessEvent(0)));
}

///////////////////////////////////////////////////////////////////////////////
// Press and release the left mouse button inside an element and check
// that mouse up, down, and click events occur.

TEST_FIXTURE(cGUIEventRouterFixture, SimplestPossibleClick)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("clickElement"), tGUIPoint(1,1), tGUISize(1,1)));

   static const sInputEvent mouseDownEvent = { kMouseLeft, kMK_None, true,  cVec2<int>(1,1), .01 };
   static const sInputEvent mouseUpEvent   = { kMouseLeft, kMK_None, false, cVec2<int>(1,1), .02 };

   HandleInputEvent(&mouseDownEvent);
   HandleInputEvent(&mouseUpEvent);

   CHECK_EQUAL(3, m_eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseDown, GUIEventCode(m_eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventMouseUp, GUIEventCode(m_eventCollector.AccessEvent(1)));
   CHECK_EQUAL(kGUIEventClick, GUIEventCode(m_eventCollector.AccessEvent(2)));
}

///////////////////////////////////////////////////////////////////////////////
// Click an element then move outside before releasing; Check that no click event occurs

TEST_FIXTURE(cGUIEventRouterFixture, CancelClick)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("cancelClickElement"), tGUIPoint(1,1), tGUISize(1,1)));

   static const sInputEvent inputEvents[] =
   {
      { kMouseMove, kMK_None, false, cVec2<int>(0,0), .01 }, // outside
      { kMouseMove, kMK_None, false, cVec2<int>(1,1), .02 }, // inside
      { kMouseLeft, kMK_None, true,  cVec2<int>(1,1), .03 }, // inside, mouse down
      { kMouseMove, kMK_None, false, cVec2<int>(0,0), .04 }, // outside, mouse still down
      { kMouseLeft, kMK_None, false, cVec2<int>(0,0), .05 }, // outside, mouse up
   };

   for (int i = 0; i < _countof(inputEvents); ++i)
   {
      HandleInputEvent(&inputEvents[i]);
   }

   CHECK_EQUAL(4, m_eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseEnter, GUIEventCode(m_eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventMouseMove, GUIEventCode(m_eventCollector.AccessEvent(1)));
   CHECK_EQUAL(kGUIEventMouseDown, GUIEventCode(m_eventCollector.AccessEvent(2)));
   CHECK_EQUAL(kGUIEventMouseLeave, GUIEventCode(m_eventCollector.AccessEvent(3)));
   // TODO: should a mouse-up occur even when the click is cancelled?
}

///////////////////////////////////////////////////////////////////////////////
// Generate a key-press event with no focus and check that no GUI event occurs

TEST_FIXTURE(cGUIEventRouterFixture, KeyEventNoFocus)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("noFocusElement"), tGUIPoint(1,1), tGUISize(1,1)));

   static const sInputEvent inputEvents[] =
   {
      { 't', kMK_None, true, cVec2<int>(0,0), .01 },
      { 't', kMK_None, false, cVec2<int>(0,0), .02 },
   };

   for (int i = 0; i < _countof(inputEvents); ++i)
   {
      HandleInputEvent(&inputEvents[i]);
   }

   CHECK_EQUAL(0, m_eventCollector.GetEventCount());
}

///////////////////////////////////////////////////////////////////////////////
// Generate a key-press event; Check that corresponding GUI events occur

TEST_FIXTURE(cGUIEventRouterFixture, KeyEventFocus)
{
   // Element at position (1,1); size 1x1 pixels
   cAutoIPtr<IGUIElement> pTestElement;
   CHECK_EQUAL(S_OK, CreateTestElement(_T("focusElement"), tGUIPoint(1,1), tGUISize(1,1), &pTestElement));

   SetFocus(pTestElement);

   static const sInputEvent inputEvents[] =
   {
      { 't', kMK_None, true, cVec2<int>(0,0), .01 },
      { 't', kMK_None, false, cVec2<int>(0,0), .02 },
   };

   for (int i = 0; i < _countof(inputEvents); ++i)
   {
      HandleInputEvent(&inputEvents[i]);
   }

   CHECK_EQUAL(2, m_eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventKeyDown, GUIEventCode(m_eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventKeyUp, GUIEventCode(m_eventCollector.AccessEvent(1)));
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP
