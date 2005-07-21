///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTFACTORYD3D_H
#define INCLUDED_GUIFONTFACTORYD3D_H

#include "guiapi.h"

#include "globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFactoryD3D
//

class cGUIFontFactoryD3D : public cComObject2<IMPLEMENTS(IGUIFontFactoryD3D), IMPLEMENTS(IGlobalObject)>
{
   cGUIFontFactoryD3D(const cGUIFontFactoryD3D &); // un-implemented
   void operator=(const cGUIFontFactoryD3D &); // un-implemented

   typedef cComObject2<IMPLEMENTS(IGUIFontFactoryD3D), IMPLEMENTS(IGlobalObject)> tBase;

public:
   cGUIFontFactoryD3D();
   ~cGUIFontFactoryD3D();

   DECLARE_NAME(GUIFontFactory)
   DECLARE_NO_CONSTRAINTS()

   // Over-ride QI to support base interface IGUIFontFactory
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid,
                                                    void * * ppvObject)
   {
      if (ppvObject == NULL)
      {
         return E_POINTER;
      }
      if (CTIsEqualGUID(iid, IID_IGUIFontFactory))
      {
         *ppvObject = CTAddRef(static_cast<IGUIFontFactory*>(this));
         return S_OK;
      }
      return tBase::QueryInterface(iid, ppvObject);
   }

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult CreateFont2(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

   virtual tResult SetD3DDevice(IDirect3DDevice9 * pDevice);

protected:
   void ReleaseAllCachedFonts();

private:
   typedef std::map<cGUIFontDesc, IGUIFont*> tFontMap;
   tFontMap m_fontMap;
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTFACTORYD3D_H
