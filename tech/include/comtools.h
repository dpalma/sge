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

template <class INTRFC, const IID * PIID, 
          class SERVICES = cDefaultComServices>
class cComObject : public INTRFC, public SERVICES
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
         { static_cast<INTRFC *>(this), PIID },
      };
      return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};

template <class INTRFC1, const IID * PIID1, 
          class INTRFC2, const IID * PIID2,
          class SERVICES = cDefaultComServices>
class cComObject2 : public INTRFC1, public INTRFC2, public SERVICES
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
         { static_cast<INTRFC1 *>(this), PIID1 },
         { static_cast<INTRFC2 *>(this), PIID2 }
      };
      return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};


///////////////////////////////////////////////////////////////////////////////
//
// cCTInterfaceMethodRef
//
// Functor objects for use with STL algorithms. For example,
//    std::for_each(objects.begin(), objects.end(), CTInterfaceMethodRef(&IUnknown::Release));

template <typename RETURN, typename INTRFC>
class cCTInterfaceMethodRef
{
   typedef RETURN (STDMETHODCALLTYPE INTRFC::*tMethod)();
public:
   explicit cCTInterfaceMethodRef(tMethod pfnMethod)
     : m_pfnMethod(pfnMethod) {}
   RETURN operator()(INTRFC * pInterface)
   {
      return (pInterface->*m_pfnMethod)();
   }
private:
   tMethod m_pfnMethod;
};

template <typename RETURN, typename INTRFC>
inline cCTInterfaceMethodRef<RETURN, INTRFC>
   CTInterfaceMethodRef(RETURN (STDMETHODCALLTYPE INTRFC::*pfnMethod)())
{
   return cCTInterfaceMethodRef<RETURN, INTRFC>(pfnMethod);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAutoIPtr
//

template <class INTRFC>
class cAutoIPtr
{
#ifdef __GNUC__
   typedef void * NullType;
#else
   typedef int NullType;
#endif

public:
   cAutoIPtr();
   cAutoIPtr(INTRFC * pI);
   cAutoIPtr(const cAutoIPtr<INTRFC> & p);

   ~cAutoIPtr();

   const cAutoIPtr<INTRFC> & operator =(NullType); // assign to NULL
   const cAutoIPtr<INTRFC> & operator =(INTRFC * pI);
   const cAutoIPtr<INTRFC> & operator =(const cAutoIPtr<INTRFC> & p);

   INTRFC ** operator &();
   INTRFC * operator ->();
   const INTRFC * operator ->() const;
   operator INTRFC *();
   operator const INTRFC *() const;
   bool operator !() const;
   bool operator !=(NullType) const; // if (p != NULL) ...

private:
   INTRFC * m_pInterface;
};

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::cAutoIPtr()
 : m_pInterface(NULL)
{
}

///////////////////////////////////////
// when receiving raw interface pointer, assume ownership of existing reference

template <class INTRFC>
inline cAutoIPtr<INTRFC>::cAutoIPtr(INTRFC * pI)
 : m_pInterface(pI)
{
}

///////////////////////////////////////
// when copying smart pointer, obtain own reference

template <class INTRFC>
inline cAutoIPtr<INTRFC>::cAutoIPtr(const cAutoIPtr<INTRFC> & p)
 : m_pInterface(p.m_pInterface)
{
   if (m_pInterface != NULL)
      m_pInterface->AddRef();
}

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::~cAutoIPtr()
{
   SafeRelease(m_pInterface);
}

///////////////////////////////////////

template <class INTRFC>
inline const cAutoIPtr<INTRFC> & cAutoIPtr<INTRFC>::operator =(NullType null)
{
   Assert(null == NULL);
   m_pInterface = NULL;
   return *this;
}

///////////////////////////////////////
// when receiving raw interface pointer, assume ownership of existing reference

template <class INTRFC>
inline const cAutoIPtr<INTRFC> & cAutoIPtr<INTRFC>::operator =(INTRFC * pI)
{
   INTRFC * pFormer = m_pInterface;
   m_pInterface = pI;
   SafeRelease(pFormer);
   return *this;
}

///////////////////////////////////////
// when copying smart pointer, obtain own reference

template <class INTRFC>
inline const cAutoIPtr<INTRFC> & cAutoIPtr<INTRFC>::operator =(const cAutoIPtr<INTRFC> & p)
{
   INTRFC * pFormer = m_pInterface;
   m_pInterface = p.m_pInterface;
   if (m_pInterface != NULL)
      m_pInterface->AddRef();
   SafeRelease(pFormer);
   return *this;
}

///////////////////////////////////////

template <class INTRFC>
inline INTRFC * * cAutoIPtr<INTRFC>::operator &()
{
   // Taking the address of the inner pointer most likely means that the pointer
   // is about to be filled in a la QueryInterface. Better not have a managed
   // pointer in that case because it could get lost.
   Assert(m_pInterface == NULL);
   return &m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline INTRFC * cAutoIPtr<INTRFC>::operator ->()
{
   Assert(m_pInterface != NULL);
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline const INTRFC * cAutoIPtr<INTRFC>::operator ->() const
{
   Assert(m_pInterface != NULL);
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::operator INTRFC *()
{
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::operator const INTRFC *() const
{
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline bool cAutoIPtr<INTRFC>::operator !() const
{
   return (m_pInterface == NULL);
}

///////////////////////////////////////

template <class INTRFC>
inline bool cAutoIPtr<INTRFC>::operator !=(NullType null) const
{
   Assert(null == NULL);
   return (m_pInterface != NULL);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COMTOOLS_H
