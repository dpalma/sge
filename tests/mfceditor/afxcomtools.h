/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_AFXCOMTOOLS_H)
#define INCLUDED_AFXCOMTOOLS_H

#include "tech/comtools.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAfxComServices
//

#define INTERFACEOFFSET(Class, Interface) \
   ((int)(((Interface *)((Class *)1))) - 1)

#define IMPLEMENTS_INTERFACE(Class, Interface) \
   IMPLEMENTS_INTERFACE_(Class, Interface, IID_##Interface)

#define IMPLEMENTS_INTERFACE_(Class, Interface, iid) \
   { &iid, INTERFACEOFFSET(Class, Interface) },

template <class CLASS>
class cAfxComServices
{
protected:
   HRESULT DoQueryInterface(const sQIPair * pPairs, int nPairs,
                            REFIID iid, void * * ppvObject)
   {
//      CLASS * pThis = static_cast<CLASS *>(this);
//      return pThis->ExternalQueryInterface(&iid, ppvObject); 
      return CTQueryInterface(pPairs, nPairs, iid, ppvObject);
   }

   ULONG DoAddRef()
   {
      CLASS * pThis = static_cast<CLASS *>(this);
      return pThis->ExternalAddRef(); 
   }

   ULONG DoRelease()
   {
      CLASS * pThis = static_cast<CLASS *>(this);
      return pThis->ExternalRelease(); 
   }
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_AFXCOMTOOLS_H)
