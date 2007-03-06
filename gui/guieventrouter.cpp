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
   virtual tResult OnEvent(IGUIEvent * pEvent);
   std::list<IGUIEvent *>::const_iterator GetFirstEvent() const { return m_events.begin(); }
   std::list<IGUIEvent *>::const_iterator GetLastEvent() const { return m_events.end(); }
private:
   typedef std::list<IGUIEvent *> tGUIEventList;
   tGUIEventList m_events;
};

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
   ForEachElement(ht);
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
      pElement->SetId("test");
   }
   pElement->SetPosition(position);
   pElement->SetSize(size);
   return AddElement(pElement);
}

///////////////////////////////////////////////////////////////////////////////

TEST_FIXTURE(cGUIEventRouterFixture, MouseEnterLeave)
{
   //   0 1 2 3
   // 0 +-+-+-+
   // 1 + + + +
   // 2 + + + +
   // 3 +-+-+-+

   static const sInputEvent inputEvents[] =
   {
      { kMouseMove, false, cVec2<int>(0, 0), .01 },
      { kMouseMove, false, cVec2<int>(1, 1), .02 },
      { kMouseMove, false, cVec2<int>(0, 0), .03 },
   };

   //static const sInputEvent inputEvents[] =
   //{
   //   { kMouseMove, false, cVec2<int>(566, 438), 17.919989 },
   //   { kMouseMove, false, cVec2<int>(564, 438), 17.933317 },
   //   { kMouseMove, false, cVec2<int>(564, 437), 17.959960 },
   //   { kMouseLeft, true, cVec2<int>(564, 437), 18.906329 },
   //   { kMouseLeft, false, cVec2<int>(564, 437), 19.053013 },
   //   { kMouseMove, false, cVec2<int>(562, 437), 19.892691 },
   //   { kMouseMove, false, cVec2<int>(560, 437), 19.933303 },
   //   { kMouseMove, false, cVec2<int>(555, 438), 20.332540 },
   //   { kMouseMove, false, cVec2<int>(553, 439), 20.372519 },
   //   { kMouseMove, false, cVec2<int>(551, 439), 20.399194 },
   //};

   CHECK_EQUAL(S_OK, CreateTestElement(_T("test"), tGUIPoint(1,1), tGUISize(1,1)));

   cStackGUIEventCollector eventCollector;
   CHECK_EQUAL(S_OK, AddEventListener(static_cast<IGUIEventListener*>(&eventCollector)));

   for (int i = 0; i < _countof(inputEvents); ++i)
   {
      HandleInputEvent(&inputEvents[i]);
   }

   CHECK_EQUAL(S_OK, RemoveEventListener(static_cast<IGUIEventListener*>(&eventCollector)));
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP
