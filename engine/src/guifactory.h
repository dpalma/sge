///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFACTORY_H
#define INCLUDED_GUIFACTORY_H

#include "guiapi.h"

#include "globalobj.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFactory
//

class cGUIFactory : public cGlobalObject<IMPLEMENTS(IGUIFactory)>
{
public:
   cGUIFactory();
   ~cGUIFactory();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult CreateElement(const char * pszType, const TiXmlElement * pXmlElement, IGUIElement * * ppElement);

   virtual tResult RegisterElementFactory(const char * pszType, IGUIElementFactory * pFactory);
   virtual tResult RevokeElementFactory(const char * pszType);

   virtual tResult RegisterElementRendererFactory(const char * pszRenderer, IGUIElementRendererFactory * pFactory);
   virtual tResult RevokeElementRendererFactory(const char * pszRenderer);

   friend tResult RegisterGUIElementFactory(const char * pszType, IGUIElementFactory * pFactory);
   friend tResult RegisterGUIElementRendererFactory(const char * pszRenderer, IGUIElementRendererFactory * pFactory);

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

   typedef std::map<cStr, IGUIElementFactory *> tGUIElementFactoryMap;
   tGUIElementFactoryMap m_elementFactoryMap;

   typedef std::map<cStr, IGUIElementRendererFactory *> tGUIRendererFactoryMap;
   tGUIRendererFactoryMap m_rendererFactoryMap;

   static bool gm_bInitialized;

   struct sElementFactoryMapEntry
   {
      char szType[200];
      IGUIElementFactory * pFactory;
      struct sElementFactoryMapEntry * pNext;
   };
   static struct sElementFactoryMapEntry * gm_pElementFactoryMapEntries;

   struct sRendererFactoryMapEntry
   {
      char szRenderer[200];
      IGUIElementRendererFactory * pFactory;
      struct sRendererFactoryMapEntry * pNext;
   };
   static struct sRendererFactoryMapEntry * gm_pRendererFactoryMapEntries;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFACTORY_H
