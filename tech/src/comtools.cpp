///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "comtools.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cstdio>
#include <cstring>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool CTIsSameObject(IUnknown * pUnk1, IUnknown * pUnk2)
{
   if (pUnk1 == pUnk2)
      return true;

   // If both are NULL, then equality test above would have already
   // returned true. This test covers one NULL and the other non-NULL.
   if (pUnk1 == NULL || pUnk2 == NULL)
   {
      return false;
   }

   IUnknown * pQueryUnk1 = NULL;
   IUnknown * pQueryUnk2 = NULL;

   pUnk1->QueryInterface(IID_IUnknown, (void**)&pQueryUnk1);
   pUnk2->QueryInterface(IID_IUnknown, (void**)&pQueryUnk2);

   bool bIsSame = false;

   if (pQueryUnk1 == pQueryUnk2)
   {
      bIsSame = true;
   }

   SafeRelease(pQueryUnk1);
   SafeRelease(pQueryUnk2);

   return bIsSame;
}

///////////////////////////////////////////////////////////////////////////////

bool cCTLessInterface::operator()(IUnknown * pUnk1, IUnknown * pUnk2) const
{
   if (pUnk1 == pUnk2)
   {
      return false;
   }

   // If both are NULL, then the test above would have already returned.
   // This test covers one NULL and the other non-NULL.
   if (pUnk1 == NULL || pUnk2 == NULL)
   {
      return (pUnk1 < pUnk2);
   }

   IUnknown * pIdentityUnk1 = NULL;
   IUnknown * pIdentityUnk2 = NULL;

   pUnk1->QueryInterface(IID_IUnknown, (void**)&pIdentityUnk1);
   pUnk2->QueryInterface(IID_IUnknown, (void**)&pIdentityUnk2);

   bool bLessThan = (pIdentityUnk1 < pIdentityUnk2);

   SafeRelease(pIdentityUnk1);
   SafeRelease(pIdentityUnk2);

   return bLessThan;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

////////////////////////////////////////

GUID IID_IFoo = 
{ 0x4e4c5299, 0x18bd, 0x4477, { 0x9c, 0x4b, 0x2b, 0x40, 0xa6, 0x98, 0x82, 0x4b } };
interface IFoo : IUnknown {};

GUID IID_IFooSideInterface = 
{ 0xdad942b, 0xd626, 0x4cfe, { 0x92, 0xe6, 0xca, 0xad, 0x88, 0xd3, 0xab, 0x29 } };
interface IFooSideInterface : IUnknown {};

GUID IID_IBar = 
{ 0x3eb94c5f, 0x2e2c, 0x4fbc, { 0xbe, 0x49, 0x1c, 0xc0, 0xc8, 0xad, 0x6f, 0xde } };
interface IBar : IUnknown {};

class cSameObjectTest1 : public cComObject2<IMPLEMENTS(IFoo), IMPLEMENTS(IFooSideInterface)> {};
class cSameObjectTest2 : public cComObject<IMPLEMENTS(IBar)> {};

////////////////////////////////////////

TEST(CTIsSameObject)
{
   cAutoIPtr<cSameObjectTest1> pObj1(new cSameObjectTest1);
   cAutoIPtr<cSameObjectTest2> pObj2(new cSameObjectTest2);

   cAutoIPtr<IFoo> pFoo;
   CHECK(pObj1->QueryInterface(IID_IFoo, (void**)&pFoo) == S_OK);

   cAutoIPtr<IFooSideInterface> pFooSide;
   CHECK(pObj1->QueryInterface(IID_IFooSideInterface, (void**)&pFooSide) == S_OK);

   CHECK(CTIsSameObject(static_cast<IFooSideInterface*>(pObj1), pFoo));
   CHECK(CTIsSameObject(static_cast<IFoo*>(pObj1), pFooSide));
   CHECK(CTIsSameObject(pFoo, pFooSide));

   CHECK(!CTIsSameObject(static_cast<IFoo*>(pObj1), pObj2));
   CHECK(!CTIsSameObject(pFoo, pObj2));
   CHECK(!CTIsSameObject(pFooSide, pObj2));
}

////////////////////////////////////////

// Implements IFoo and aggregates IBar using cAggTestInner
class cAggTest : public cComObject<IMPLEMENTS(IFoo)>
{
   tResult InitBar();
public:
   static tResult Create(IFoo * * ppFoo);
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid, void * * ppvObject);
private:
   cAutoIPtr<IUnknown> m_pUnkBar;
   cAutoIPtr<IBar> m_pBar;
};

class cAggTestInner : public cComAggregableObject<IMPLEMENTS(IBar)>
{
   cAggTestInner(IUnknown * pUnkOuter);
public:
   static tResult Create(IUnknown * pUnkOuter, IUnknown * * ppInner);
};

tResult cAggTest::InitBar()
{
   if (!!m_pBar || !!m_pUnkBar)
   {
      return E_FAIL;
   }

   if (cAggTestInner::Create(this, &m_pUnkBar) != S_OK)
   {
      return E_FAIL;
   }

   if (m_pUnkBar->QueryInterface(IID_IBar, (void**)&m_pBar) != S_OK)
   {
      m_pUnkBar->Release();
      return E_FAIL;
   }

   // Release reference from QI above
   Release();
   return S_OK;
}

tResult cAggTest::Create(IFoo * * ppFoo)
{
   if (ppFoo == NULL)
   {
      return E_POINTER;
   }

   cAggTest * pAggTest = new cAggTest;
   if (pAggTest == NULL)
   {
      return E_OUTOFMEMORY;
   }

   if (pAggTest->InitBar() != S_OK)
   {
      delete pAggTest;
      return E_FAIL;
   }

   *ppFoo = static_cast<IFoo*>(pAggTest);
   return S_OK;
}

tResult STDMETHODCALLTYPE cAggTest::QueryInterface(REFGUID iid, void * * ppvObject)
{
   const struct sQIPair pairs[] =
   {
      { static_cast<IFoo *>(this), &IID_IFoo },
      { static_cast<IBar *>(m_pBar), &IID_IBar },
   };
   return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
}

cAggTestInner::cAggTestInner(IUnknown * pUnkOuter)
 : cComAggregableObject<IMPLEMENTS(IBar)>(pUnkOuter)
{
}

tResult cAggTestInner::Create(IUnknown * pUnkOuter, IUnknown * * ppInner)
{
   if (ppInner == NULL)
   {
      return E_POINTER;
   }
   cAggTestInner * pInner = new cAggTestInner(pUnkOuter);
   if (pInner == NULL)
   {
      return E_OUTOFMEMORY;
   }
   *ppInner = pInner->AccessInnerUnknown();
   return S_OK;
}

////////////////////////////////////////

TEST(ComToolsAggregation)
{
   // "foo" that aggregates "bar"
   {
      cAutoIPtr<IFoo> pFoo;
      CHECK(cAggTest::Create(&pFoo) == S_OK);

      cAutoIPtr<IBar> pBar;
      CHECK(pFoo->QueryInterface(IID_IBar, (void**)&pBar) == S_OK);

      cAutoIPtr<IFoo> pFoo2;
      CHECK(pBar->QueryInterface(IID_IFoo, (void**)&pFoo2) == S_OK);

      CHECK(CTIsSameObject(pFoo, pBar));
   }

   // "bar" with no aggregation
   {
      cAutoIPtr<IUnknown> pUnkBar;
      CHECK(cAggTestInner::Create(NULL, &pUnkBar) == S_OK);

      cAutoIPtr<IBar> pBar;
      CHECK(pUnkBar->QueryInterface(IID_IBar, (void**)&pBar) == S_OK);
   }
}


///////////////////////////////////////////////////////////////////////////////

class cDummy : public cComObject<IMPLEMENTS(IUnknown)>
{
public:
   cDummy() { gm_nDummies++; }
   ~cDummy() { gm_nDummies--; }
   long ConstCall(long n) const { return n; }
   long NonConstCall(long n) { return n; }
   static int GetObjectCount() { return gm_nDummies; }
private:
   static int gm_nDummies;
};

int cDummy::gm_nDummies = 0;

void MakeDummy(cDummy * * ppDummy)
{
   *ppDummy = new cDummy;
}

TEST(AutoIPtrAssignExisting)
{
   CHECK(cDummy::GetObjectCount() == 0);
   cDummy * pDummy = new cDummy;
   CHECK(cDummy::GetObjectCount() == 1);
   {
      cAutoIPtr<IUnknown> pDummy2;
      pDummy2 = pDummy;
   }
   CHECK(cDummy::GetObjectCount() == 0);
}

TEST(AutoIPtrInitialized)
{
   CHECK(cDummy::GetObjectCount() == 0);
   {
      cAutoIPtr<IUnknown> pDummy = new cDummy;
      CHECK(cDummy::GetObjectCount() == 1);
   }
   CHECK(cDummy::GetObjectCount() == 0);
}

TEST(AutoIPtrNull)
{
   cAutoIPtr<IUnknown> pDummy;
   CHECK(!pDummy);
   CHECK(!(pDummy != NULL));
}

TEST(AutoIPtrCopy)
{
   CHECK(cDummy::GetObjectCount() == 0);
   cDummy * pDummy = new cDummy;
   CHECK(cDummy::GetObjectCount() == 1);
   {
      cAutoIPtr<IUnknown> pDummy2 = pDummy;
      {
         cAutoIPtr<IUnknown> pDummy3 = pDummy2;
      }
   }
   CHECK(cDummy::GetObjectCount() == 0);
}

TEST(AutoIPtrSafeRelease)
{
   CHECK(cDummy::GetObjectCount() == 0);
   cAutoIPtr<IUnknown> pDummy = new cDummy;
   CHECK(cDummy::GetObjectCount() == 1);
   SafeRelease(pDummy);
   CHECK(cDummy::GetObjectCount() == 0);
}

TEST(AutoIPtrCall)
{
   CHECK(cDummy::GetObjectCount() == 0);
   {
      cAutoIPtr<cDummy> pDummy = new cDummy;
      CHECK(cDummy::GetObjectCount() == 1);
      CHECK(pDummy->ConstCall(0xDEADBEEF) == 0xDEADBEEF);
      CHECK(pDummy->NonConstCall(0xDEADBEEF) == 0xDEADBEEF);
   }
   CHECK(cDummy::GetObjectCount() == 0);
}

TEST(AutoIPtrAddressOf)
{
   CHECK(cDummy::GetObjectCount() == 0);
   {
      cAutoIPtr<cDummy> pDummy;
      MakeDummy(&pDummy);
      CHECK(cDummy::GetObjectCount() == 1);
   }
   CHECK(cDummy::GetObjectCount() == 0);
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
