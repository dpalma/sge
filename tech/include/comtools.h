///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COMTOOLS_H
#define INCLUDED_COMTOOLS_H

#include "techdll.h"
#include "combase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

TECH_API bool GUIDToString(REFGUID guid, char * psz, int maxLen);

///////////////////////////////////////

inline BOOL CTIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return (
	  ((long*) &rguid1)[0] == ((long*) &rguid2)[0] &&
	  ((long*) &rguid1)[1] == ((long*) &rguid2)[1] &&
	  ((long*) &rguid1)[2] == ((long*) &rguid2)[2] &&
	  ((long*) &rguid1)[3] == ((long*) &rguid2)[3]);
}

inline BOOL CTIsEqualUnknown(REFGUID rguid)
{
   return CTIsEqualGUID(rguid, IID_IUnknown);
}

///////////////////////////////////////

template <class T>
ulong SafeRelease(T & p)
{
   if (!!p)
   {
      ulong result = p->Release();
      (void * &)p = NULL;
      return result;
   }
   return (ulong)-1;
}

///////////////////////////////////////////////////////////////////////////////

inline bool CTIsSameObject(IUnknown * pUnk1, IUnknown * pUnk2)
{
   if (pUnk1 == pUnk2)
      return true;

   IUnknown * pQueryUnk1 = NULL;
   IUnknown * pQueryUnk2 = NULL;

   pUnk1->QueryInterface(IID_IUnknown, (void**)&pQueryUnk1);
   pUnk2->QueryInterface(IID_IUnknown, (void**)&pQueryUnk2);

   bool bIsSame = false;

   if (pUnk1 == pUnk2)
      bIsSame = true;

   SafeRelease(pQueryUnk1);
   SafeRelease(pQueryUnk2);

   return bIsSame;
}

///////////////////////////////////////////////////////////

struct sQIPair
{
   IUnknown * pUnk;
   const IID * pIID;
};

inline HRESULT CTQueryInterface(const sQIPair * pPairs, int nPairs,
                                REFIID iid, void * * ppvObject)
{
   for (int i = 0; i < nPairs; i++)
   {
      if (CTIsEqualGUID(iid, *pPairs[i].pIID))
      {
         pPairs[i].pUnk->AddRef();
         *ppvObject = pPairs[i].pUnk;
         return S_OK;
      }
   }
   if (CTIsEqualUnknown(iid))
   {
      pPairs[0].pUnk->AddRef();
      *ppvObject = pPairs[0].pUnk;
      return S_OK;
   }
   *ppvObject = NULL;
   return E_NOINTERFACE;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNonDelegatingComServices
//

class cNonDelegatingComServices
{
protected:
   cNonDelegatingComServices()
     : m_cRef(1)
   {
   }

   virtual ~cNonDelegatingComServices()
   {
   }

   HRESULT DoQueryInterface(const sQIPair * pPairs, int nPairs,
                            REFIID iid, void * * ppvObject)
   {
      return CTQueryInterface(pPairs, nPairs, iid, ppvObject);
   }

   ULONG DoAddRef()
   {
      return ++m_cRef;
   }

   ULONG DoRelease()
   {
      if (--m_cRef)
         return m_cRef;
      OnFinalRelease();
      DeleteThis();
      return 0;
   }

   virtual void OnFinalRelease()
   {
   }

   virtual void DeleteThis()
   {
      delete this;
   }

private:
   ULONG m_cRef;
};

typedef cNonDelegatingComServices cDefaultComServices;


///////////////////////////////////////////////////////////////////////////////
//
// COM object base classes
//
// Example:
//    class cFoo : public cComObject<IMPLEMENTS(IFoo)>
//    {
//    public:
//    };

#define IMPLEMENTS(Interface) Interface, &IID_##Interface

template <class INTERFACE, const IID * PIID, 
          class SERVICES = cDefaultComServices>
class cComObject : public INTERFACE, public SERVICES
{
public:
   virtual ~cComObject() {}
   virtual ULONG STDMETHODCALLTYPE AddRef() { return DoAddRef(); }
   virtual ULONG STDMETHODCALLTYPE Release() { return DoRelease(); }
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<INTERFACE *>(this), PIID },
      };
      return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};

template <class INTERFACE1, const IID * PIID1, 
          class INTERFACE2, const IID * PIID2,
          class SERVICES = cDefaultComServices>
class cComObject2 : public INTERFACE1, public INTERFACE2, public SERVICES
{
public:
   virtual ~cComObject2() {}
   virtual ULONG STDMETHODCALLTYPE AddRef() { return DoAddRef(); }
   virtual ULONG STDMETHODCALLTYPE Release() { return DoRelease(); }
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<INTERFACE1 *>(this), PIID1 },
         { static_cast<INTERFACE2 *>(this), PIID2 }
      };
      return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAutoIPtr
//

template <class INTERFACE>
class cAutoIPtr
{
#ifdef __GNUC__
   typedef void * NullType;
#else
   typedef int NullType;
#endif

public:
   cAutoIPtr();
   cAutoIPtr(INTERFACE * pI);
   cAutoIPtr(const cAutoIPtr<INTERFACE> & p);

   ~cAutoIPtr();

   const cAutoIPtr<INTERFACE> & operator =(NullType); // assign to NULL
   const cAutoIPtr<INTERFACE> & operator =(INTERFACE * pI);
   const cAutoIPtr<INTERFACE> & operator =(const cAutoIPtr<INTERFACE> & p);

   INTERFACE ** operator &();
   INTERFACE * operator ->();
   const INTERFACE * operator ->() const;
   operator INTERFACE *();
   operator const INTERFACE *() const;
   bool operator !() const;
   bool operator !=(NullType) const; // if (p != NULL) ...

private:
   INTERFACE * m_pInterface;
};

///////////////////////////////////////

template <class INTERFACE>
inline cAutoIPtr<INTERFACE>::cAutoIPtr()
 : m_pInterface(NULL)
{
}

///////////////////////////////////////
// when receiving raw interface pointer, assume ownership of existing reference

template <class INTERFACE>
inline cAutoIPtr<INTERFACE>::cAutoIPtr(INTERFACE * pI)
 : m_pInterface(pI)
{
}

///////////////////////////////////////
// when copying smart pointer, obtain own reference

template <class INTERFACE>
inline cAutoIPtr<INTERFACE>::cAutoIPtr(const cAutoIPtr<INTERFACE> & p)
 : m_pInterface(p.m_pInterface)
{
   if (m_pInterface != NULL)
      m_pInterface->AddRef();
}

///////////////////////////////////////

template <class INTERFACE>
inline cAutoIPtr<INTERFACE>::~cAutoIPtr()
{
   SafeRelease(m_pInterface);
}

///////////////////////////////////////

template <class INTERFACE>
inline const cAutoIPtr<INTERFACE> & cAutoIPtr<INTERFACE>::operator =(NullType null)
{
   Assert(null == NULL);
   m_pInterface = NULL;
   return *this;
}

///////////////////////////////////////
// when receiving raw interface pointer, assume ownership of existing reference

template <class INTERFACE>
inline const cAutoIPtr<INTERFACE> & cAutoIPtr<INTERFACE>::operator =(INTERFACE * pI)
{
   INTERFACE * pFormer = m_pInterface;
   m_pInterface = pI;
   SafeRelease(pFormer);
   return *this;
}

///////////////////////////////////////
// when copying smart pointer, obtain own reference

template <class INTERFACE>
inline const cAutoIPtr<INTERFACE> & cAutoIPtr<INTERFACE>::operator =(const cAutoIPtr<INTERFACE> & p)
{
   INTERFACE * pFormer = m_pInterface;
   m_pInterface = p.m_pInterface;
   if (m_pInterface != NULL)
      m_pInterface->AddRef();
   SafeRelease(pFormer);
   return *this;
}

///////////////////////////////////////

template <class INTERFACE>
inline INTERFACE ** cAutoIPtr<INTERFACE>::operator &()
{
   // Taking the address of the inner pointer most likely means that the pointer
   // is about to be filled in a la QueryInterface. Better not have a managed
   // pointer in that case because it could get lost.
   Assert(m_pInterface == NULL);
   return &m_pInterface;
}

///////////////////////////////////////

template <class INTERFACE>
inline INTERFACE * cAutoIPtr<INTERFACE>::operator ->()
{
   Assert(m_pInterface != NULL);
   return m_pInterface;
}

///////////////////////////////////////

template <class INTERFACE>
inline const INTERFACE * cAutoIPtr<INTERFACE>::operator ->() const
{
   Assert(m_pInterface != NULL);
   return m_pInterface;
}

///////////////////////////////////////

template <class INTERFACE>
inline cAutoIPtr<INTERFACE>::operator INTERFACE *()
{
   return m_pInterface;
}

///////////////////////////////////////

template <class INTERFACE>
inline cAutoIPtr<INTERFACE>::operator const INTERFACE *() const
{
   return m_pInterface;
}

///////////////////////////////////////

template <class INTERFACE>
inline bool cAutoIPtr<INTERFACE>::operator !() const
{
   return (m_pInterface == NULL);
}

///////////////////////////////////////

template <class INTERFACE>
inline bool cAutoIPtr<INTERFACE>::operator !=(NullType null) const
{
   Assert(null == NULL);
   return (m_pInterface != NULL);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COMTOOLS_H
