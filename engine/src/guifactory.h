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

   friend tResult RegisterGUIElementFactory(const char * pszType, IGUIElementFactory * pFactory);

private:
   typedef std::map<cStr, IGUIElementFactory *> tGUIElementFactoryMap;
   tGUIElementFactoryMap m_elementFactoryMap;

   struct sElementFactoryMapEntry
   {
      char szType[200];
      IGUIElementFactory * pFactory;
      struct sElementFactoryMapEntry * pNext;
   };
   static struct sElementFactoryMapEntry * gm_pElementFactoryMapEntries;
   static bool gm_bInitialized;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFACTORY_H
