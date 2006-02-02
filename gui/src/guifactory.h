///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFACTORY_H
#define INCLUDED_GUIFACTORY_H

#include "guiapi.h"

#include "connptimpl.h"
#include "globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFactory
//

class cGUIFactory : public cComObject2<IMPLEMENTSCP(IGUIFactory, IGUIFactoryListener),
                                       IMPLEMENTS(IGlobalObject)>
{
   typedef cConnectionPoint<IGUIFactory, IGUIFactoryListener> tCP;

public:
   cGUIFactory();
   ~cGUIFactory();

   DECLARE_NAME(GUIFactory)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddFactoryListener(IGUIFactoryListener * pListener);
   virtual tResult RemoveFactoryListener(IGUIFactoryListener * pListener);

   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * * ppElement);
   virtual tResult CreateRenderer(const tChar * pszRendererClass, IGUIElementRenderer * * ppRenderer);

   virtual tResult RegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn);
   virtual tResult RevokeElementFactory(const tChar * pszType);

   virtual tResult RegisterRendererFactory(const tChar * pszRenderer, tGUIRendererFactoryFn pFactoryFn);
   virtual tResult RevokeRendererFactory(const tChar * pszRenderer);

   friend tResult GUIRegisterElementFactory(const tChar * pszType, tGUIElementFactoryFn pFactoryFn);
   friend tResult GUIRegisterRendererFactory(const tChar * pszRenderer, tGUIRendererFactoryFn pFactoryFn);

private:
   void CleanupElementFactories();
   void CleanupRendererFactories();

   class cAutoCleanupStatics
   {
   public:
      ~cAutoCleanupStatics();
   };
   friend class cAutoCleanupStatics;
   static cAutoCleanupStatics g_autoCleanupStatics;

   typedef std::map<cStr, tGUIElementFactoryFn> tGUIElementFactoryFnMap;
   tGUIElementFactoryFnMap m_elementFactoryFnMap;

   typedef std::map<cStr, tGUIRendererFactoryFn> tGUIRendererFactoryFnMap;
   tGUIRendererFactoryFnMap m_rendererFactoryFnMap;

   static bool gm_bInitialized;

   struct sElementFactoryFnNode
   {
      cStr type;
      tGUIElementFactoryFn pFactoryFn;
      struct sElementFactoryFnNode * pNext;
   };

   typedef struct sElementFactoryFnNode tElementFactoryFnNode;
   static tElementFactoryFnNode * gm_pElementFactoryFns;

   struct sRendererFactoryFnNode
   {
      cStr renderer;
      tGUIRendererFactoryFn pFactoryFn;
      struct sRendererFactoryFnNode * pNext;
   };

   typedef struct sRendererFactoryFnNode tRendererFactoryFnNode;
   static tRendererFactoryFnNode * gm_pRendererFactoryFns;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFACTORY_H
