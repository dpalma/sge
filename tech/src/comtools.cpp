///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "comtools.h"
#include "techstring.h"

#include <cstdio>
#include <cstring>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool GUIDToString(REFGUID guid, tChar * psz, int maxLen)
{
   cStr temp;
   int result = temp.Format(_T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
      guid.Data1, guid.Data2, guid.Data3,
      guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
      guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
   if (result > 0)
   {
      if (result < maxLen)
      {
         _tcscpy(psz, temp.c_str());
         psz[result] = 0;
         return true;
      }
   }
   return false;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cComToolsTests : public CppUnit::TestCase
{
   void TestGuidToString();
   void TestIsSameObject();
   void TestAggregation();

   static const GUID IID_IFoo;
   static const GUID IID_IFooSideInterface;
   static const GUID IID_IBar;

   interface IFoo : IUnknown {};
   interface IFooSideInterface : IUnknown {};
   interface IBar : IUnknown {};

   class cSameObjectTest1 : public cComObject2<IMPLEMENTS(IFoo), IMPLEMENTS(IFooSideInterface)> {};
   class cSameObjectTest2 : public cComObject<IMPLEMENTS(IBar)> {};

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

   CPPUNIT_TEST_SUITE(cComToolsTests);
      CPPUNIT_TEST(TestGuidToString);
      CPPUNIT_TEST(TestIsSameObject);
      CPPUNIT_TEST(TestAggregation);
   CPPUNIT_TEST_SUITE_END();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cComToolsTests);

////////////////////////////////////////

const GUID cComToolsTests::IID_IFoo = 
{ 0x4e4c5299, 0x18bd, 0x4477, { 0x9c, 0x4b, 0x2b, 0x40, 0xa6, 0x98, 0x82, 0x4b } };
const GUID cComToolsTests::IID_IFooSideInterface = 
{ 0xdad942b, 0xd626, 0x4cfe, { 0x92, 0xe6, 0xca, 0xad, 0x88, 0xd3, 0xab, 0x29 } };
const GUID cComToolsTests::IID_IBar = 
{ 0x3eb94c5f, 0x2e2c, 0x4fbc, { 0xbe, 0x49, 0x1c, 0xc0, 0xc8, 0xad, 0x6f, 0xde } };

////////////////////////////////////////

void cComToolsTests::TestGuidToString()
{
   // {B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}
   static const GUID kTestGuidA = {0xb40b6831, 0xfcb2, 0x4082, {0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb}};
   static const GUID kTestGuidADupe = {0xb40b6831, 0xfcb2, 0x4082, {0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb}};

   // {DED26BAE-E83F-4c59-8B95-A309311B15A9}
   static const GUID kTestGuidB = {0xded26bae, 0xe83f, 0x4c59, {0x8b, 0x95, 0xa3, 0x9, 0x31, 0x1b, 0x15, 0xa9}};

   char sz[kGuidStringLength];
   CPPUNIT_ASSERT(GUIDToString(kTestGuidA, sz, _countof(sz)));
   CPPUNIT_ASSERT(strcmp(sz, "{B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}") == 0);

   char szTooSmall[kGuidStringLength / 4];
   CPPUNIT_ASSERT(!GUIDToString(kTestGuidA, szTooSmall, _countof(szTooSmall)));

   CPPUNIT_ASSERT(CTIsEqualGUID(kTestGuidA, kTestGuidADupe));
   CPPUNIT_ASSERT(!CTIsEqualGUID(kTestGuidA, kTestGuidB));
}

////////////////////////////////////////

void cComToolsTests::TestIsSameObject()
{
   cAutoIPtr<cSameObjectTest1> pObj1(new cSameObjectTest1);
   cAutoIPtr<cSameObjectTest2> pObj2(new cSameObjectTest2);

   cAutoIPtr<IFoo> pFoo;
   CPPUNIT_ASSERT(pObj1->QueryInterface(IID_IFoo, (void**)&pFoo) == S_OK);

   cAutoIPtr<IFooSideInterface> pFooSide;
   CPPUNIT_ASSERT(pObj1->QueryInterface(IID_IFooSideInterface, (void**)&pFooSide) == S_OK);

   CPPUNIT_ASSERT(CTIsSameObject(static_cast<IFooSideInterface*>(pObj1), pFoo));
   CPPUNIT_ASSERT(CTIsSameObject(static_cast<IFoo*>(pObj1), pFooSide));
   CPPUNIT_ASSERT(CTIsSameObject(pFoo, pFooSide));

   CPPUNIT_ASSERT(!CTIsSameObject(static_cast<IFoo*>(pObj1), pObj2));
   CPPUNIT_ASSERT(!CTIsSameObject(pFoo, pObj2));
   CPPUNIT_ASSERT(!CTIsSameObject(pFooSide, pObj2));

}

////////////////////////////////////////

tResult cComToolsTests::cAggTest::InitBar()
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

tResult cComToolsTests::cAggTest::Create(IFoo * * ppFoo)
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

tResult STDMETHODCALLTYPE cComToolsTests::cAggTest::QueryInterface(REFGUID iid, void * * ppvObject)
{
   const struct sQIPair pairs[] =
   {
      { static_cast<IFoo *>(this), &IID_IFoo },
      { static_cast<IBar *>(m_pBar), &IID_IBar },
   };
   return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
}

cComToolsTests::cAggTestInner::cAggTestInner(IUnknown * pUnkOuter)
 : cComAggregableObject<IMPLEMENTS(IBar)>(pUnkOuter)
{
}

tResult cComToolsTests::cAggTestInner::Create(IUnknown * pUnkOuter, IUnknown * * ppInner)
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

void cComToolsTests::TestAggregation()
{
   // "foo" that aggregates "bar"
   {
      cAutoIPtr<IFoo> pFoo;
      CPPUNIT_ASSERT(cAggTest::Create(&pFoo) == S_OK);

      cAutoIPtr<IBar> pBar;
      CPPUNIT_ASSERT(pFoo->QueryInterface(IID_IBar, (void**)&pBar) == S_OK);

      cAutoIPtr<IFoo> pFoo2;
      CPPUNIT_ASSERT(pBar->QueryInterface(IID_IFoo, (void**)&pFoo2) == S_OK);

      CPPUNIT_ASSERT(CTIsSameObject(pFoo, pBar));
   }

   // "bar" with no aggregation
   {
      cAutoIPtr<IUnknown> pUnkBar;
      CPPUNIT_ASSERT(cAggTestInner::Create(NULL, &pUnkBar) == S_OK);

      cAutoIPtr<IBar> pBar;
      CPPUNIT_ASSERT(pUnkBar->QueryInterface(IID_IBar, (void**)&pBar) == S_OK);
   }
}


///////////////////////////////////////////////////////////////////////////////

class cAutoIPtrTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cAutoIPtrTests);
      CPPUNIT_TEST(TestAutoIPtrAssignExisting);
      CPPUNIT_TEST(TestAutoIPtrInitialized);
      CPPUNIT_TEST(TestAutoIPtrNull);
      CPPUNIT_TEST(TestAutoIPtrCopy);
      CPPUNIT_TEST(TestAutoIPtrSafeRelease);
      CPPUNIT_TEST(TestAutoIPtrCall);
      CPPUNIT_TEST(TestAutoIPtrAddressOf);
   CPPUNIT_TEST_SUITE_END();

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

   void MakeDummy(cDummy * * ppDummy);

   void TestAutoIPtrAssignExisting();
   void TestAutoIPtrInitialized();
   void TestAutoIPtrNull();
   void TestAutoIPtrCopy();
   void TestAutoIPtrSafeRelease();
   void TestAutoIPtrCall();
   void TestAutoIPtrAddressOf();
};

int cAutoIPtrTests::cDummy::gm_nDummies = 0;

void cAutoIPtrTests::MakeDummy(cDummy * * ppDummy)
{
   *ppDummy = new cDummy;
}

void cAutoIPtrTests::TestAutoIPtrAssignExisting()
{
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
   cDummy * pDummy = new cDummy;
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 1);
   {
      cAutoIPtr<IUnknown> pDummy2;
      pDummy2 = pDummy;
   }
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
}

void cAutoIPtrTests::TestAutoIPtrInitialized()
{
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
   {
      cAutoIPtr<IUnknown> pDummy = new cDummy;
      CPPUNIT_ASSERT(cDummy::GetObjectCount() == 1);
   }
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
}

void cAutoIPtrTests::TestAutoIPtrNull()
{
   cAutoIPtr<IUnknown> pDummy;
   CPPUNIT_ASSERT(!pDummy);
   CPPUNIT_ASSERT(!(pDummy != NULL));
}

void cAutoIPtrTests::TestAutoIPtrCopy()
{
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
   cDummy * pDummy = new cDummy;
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 1);
   {
      cAutoIPtr<IUnknown> pDummy2 = pDummy;
      {
         cAutoIPtr<IUnknown> pDummy3 = pDummy2;
      }
   }
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
}

void cAutoIPtrTests::TestAutoIPtrSafeRelease()
{
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
   cAutoIPtr<IUnknown> pDummy = new cDummy;
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 1);
   SafeRelease(pDummy);
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
}

void cAutoIPtrTests::TestAutoIPtrCall()
{
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
   {
      cAutoIPtr<cDummy> pDummy = new cDummy;
      CPPUNIT_ASSERT(cDummy::GetObjectCount() == 1);
      CPPUNIT_ASSERT(pDummy->ConstCall(0xDEADBEEF) == 0xDEADBEEF);
      CPPUNIT_ASSERT(pDummy->NonConstCall(0xDEADBEEF) == 0xDEADBEEF);
   }
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
}

void cAutoIPtrTests::TestAutoIPtrAddressOf()
{
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
   {
      cAutoIPtr<cDummy> pDummy;
      MakeDummy(&pDummy);
      CPPUNIT_ASSERT(cDummy::GetObjectCount() == 1);
   }
   CPPUNIT_ASSERT(cDummy::GetObjectCount() == 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(cAutoIPtrTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
