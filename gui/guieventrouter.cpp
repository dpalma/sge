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

#include "UnitTest++.h"

#include "tech/dbgalloc.h" // must be last header


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
                                                                 cGUIContainerBase<IGUIContainerElement> >,
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

   tResult GetHitElement(const tScreenPoint & point, IGUIElement * * ppElement) const;

   virtual void DeleteThis() {}

   tResult CreateTestElement(const tChar * pszId, const tGUIPoint & position, const tGUISize & size);

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
};

////////////////////////////////////////

cGUIEventRouterFixture::cGUIEventRouterFixture()
{
}

////////////////////////////////////////

cGUIEventRouterFixture::~cGUIEventRouterFixture()
{
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

////////////////////////////////////////

tResult cGUIEventRouterFixture::CreateTestElement(const tChar * pszId, const tGUIPoint & position, const tGUISize & size)
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
   return AddElement(pElement);
}

///////////////////////////////////////////////////////////////////////////////
// Move the mouse inside then back outside of an element and check
// that a mouse enter and leave event were dispatched.

TEST_FIXTURE(cGUIEventRouterFixture, MouseEnterLeave)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("mouseEnterLeaveElement"), tGUIPoint(1,1), tGUISize(1,1)));

   cStackGUIEventCollector eventCollector;
   CHECK_EQUAL(S_OK, AddEventListener(static_cast<IGUIEventListener*>(&eventCollector)));

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

   CHECK_EQUAL(2, eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseEnter, GUIEventCode(eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventMouseLeave, GUIEventCode(eventCollector.AccessEvent(1)));

   CHECK_EQUAL(S_OK, RemoveEventListener(static_cast<IGUIEventListener*>(&eventCollector)));
}

///////////////////////////////////////////////////////////////////////////////
// Press and release the left mouse button inside an element and check
// that mouse up, down, and click events occur.

TEST_FIXTURE(cGUIEventRouterFixture, SimplestPossibleClick)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("clickElement"), tGUIPoint(1,1), tGUISize(1,1)));

   cStackGUIEventCollector eventCollector;
   CHECK_EQUAL(S_OK, AddEventListener(static_cast<IGUIEventListener*>(&eventCollector)));

   static const sInputEvent mouseDownEvent = { kMouseLeft, kMK_None, true,  cVec2<int>(1,1), .01 };
   static const sInputEvent mouseUpEvent   = { kMouseLeft, kMK_None, false, cVec2<int>(1,1), .02 };

   HandleInputEvent(&mouseDownEvent);
   HandleInputEvent(&mouseUpEvent);

   CHECK_EQUAL(3, eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseDown, GUIEventCode(eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventMouseUp, GUIEventCode(eventCollector.AccessEvent(1)));
   CHECK_EQUAL(kGUIEventClick, GUIEventCode(eventCollector.AccessEvent(2)));

   CHECK_EQUAL(S_OK, RemoveEventListener(static_cast<IGUIEventListener*>(&eventCollector)));
}

///////////////////////////////////////////////////////////////////////////////
// Click an element then move outside before releasing; Check that no click event occurs

TEST_FIXTURE(cGUIEventRouterFixture, CancelClick)
{
   // Element at position (1,1); size 1x1 pixels
   CHECK_EQUAL(S_OK, CreateTestElement(_T("cancelClickElement"), tGUIPoint(1,1), tGUISize(1,1)));

   cStackGUIEventCollector eventCollector;
   CHECK_EQUAL(S_OK, AddEventListener(static_cast<IGUIEventListener*>(&eventCollector)));

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

   CHECK_EQUAL(3, eventCollector.GetEventCount());
   CHECK_EQUAL(kGUIEventMouseEnter, GUIEventCode(eventCollector.AccessEvent(0)));
   CHECK_EQUAL(kGUIEventMouseDown, GUIEventCode(eventCollector.AccessEvent(1)));
   CHECK_EQUAL(kGUIEventMouseLeave, GUIEventCode(eventCollector.AccessEvent(2)));
   // TODO: should a mouse-up occur even when no element is hit?

   CHECK_EQUAL(S_OK, RemoveEventListener(static_cast<IGUIEventListener*>(&eventCollector)));
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP
