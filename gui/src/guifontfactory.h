///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTFACTORY_H
#define INCLUDED_GUIFONTFACTORY_H

#include "guiapi.h"

#include "globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFactory
//

class cGUIFontFactory : public cComObject2<IMPLEMENTS(IGUIFontFactory), IMPLEMENTS(IGlobalObject)>
{
   cGUIFontFactory(const cGUIFontFactory &); // un-implemented
   void operator=(const cGUIFontFactory &); // un-implemented

public:
   cGUIFontFactory();
   ~cGUIFontFactory();

   DECLARE_NAME(GUIFontFactory)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult CreateFontA(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);
   virtual tResult CreateFontW(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

#if HAVE_DIRECTX
   virtual tResult SetDirect3DDevice(IDirect3DDevice9 * pD3dDevice);
#endif

private:
   typedef std::map<cGUIFontDesc, IGUIFont*, std::less<cGUIFontDesc> > tFontMap;
   tFontMap m_fontMap;

#if HAVE_DIRECTX
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTFACTORY_H
