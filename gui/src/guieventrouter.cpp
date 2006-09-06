///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

LOG_DEFINE_CHANNEL(GUIEventRouter);

#ifdef HAVE_UNITTESTPP

#include "guieventrouter.h"
#include "guieventroutertem.h"

#include "guicontainerbase.h"
#include "guicontainerbasetem.h"

#include "inputapi.h"

#include "globalobj.h"

#include "UnitTest++.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIElementForTesting
//

class cGUIElementForTesting : public cComObject<cGUIElementBase<IGUIElement>, &IID_IGUIElement>
{
public:
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEventListenerForTesting
//

class cGUIEventListenerForTesting : public cComObject<IMPLEMENTS(IGUIEventListener)>
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
   virtual void DeleteThis() {}

   virtual tResult OnEvent(IGUIEvent * pEvent);
};

////////////////////////////////////////

tResult cGUIEventListenerForTesting::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}


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

   void AddElementsForTesting();

   virtual void DeleteThis() {}

private:
   cGUIEventListenerForTesting m_guiEventListener;
};

////////////////////////////////////////

cGUIEventRouterFixture::cGUIEventRouterFixture()
{
   Verify(AddEventListener(static_cast<IGUIEventListener*>(&m_guiEventListener)) == S_OK);
   AddElementsForTesting();
}

////////////////////////////////////////

cGUIEventRouterFixture::~cGUIEventRouterFixture()
{
   Verify(RemoveEventListener(static_cast<IGUIEventListener*>(&m_guiEventListener)) == S_OK);
}

////////////////////////////////////////

void cGUIEventRouterFixture::AddElementsForTesting()
{
   for (int i = 0; i < 3; i++)
   {
      cAutoIPtr<IGUIElement> pElement(static_cast<IGUIElement*>(new cGUIElementForTesting));
      if (!!pElement)
      {
         cStr id;
         Sprintf(&id, "TestElement%d", i);
         pElement->SetId(id.c_str());
         Verify(AddElement(pElement) == S_OK);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP
