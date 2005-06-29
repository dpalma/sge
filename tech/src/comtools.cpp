///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "comtools.h"

#include <cstdio>
#include <cstring>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGuid
//

struct sGuid
{
   unsigned long Data1;
   unsigned short Data2;
   unsigned short Data3;
   unsigned char Data4[8];
};

typedef struct _GUID GUID;

class cGuid : public sGuid
{
   // private, un-implemented
   cGuid(const cGuid &);
   const cGuid & operator =(const cGuid &);

public:
   cGuid(unsigned long, unsigned short, unsigned short,
      unsigned char, unsigned char, unsigned char, unsigned char,
      unsigned char, unsigned char, unsigned char, unsigned char);

   bool operator ==(const cGuid & other) const;
   bool operator !=(const cGuid & other) const;

   bool ToString(char * psz, int maxLen) const;

   static cGuid Null;
};

///////////////////////////////////////

cGuid cGuid::Null(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

///////////////////////////////////////

cGuid::cGuid(unsigned long l, unsigned short w1, unsigned short w2,
             unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4,
             unsigned char b5, unsigned char b6, unsigned char b7, unsigned char b8)
{
   Data1 = l;
   Data2 = w1;
   Data3 = w2;
   Data4[0] = b1;
   Data4[1] = b2;
   Data4[2] = b3;
   Data4[3] = b4;
   Data4[4] = b5;
   Data4[5] = b6;
   Data4[6] = b7;
   Data4[7] = b8;
}

///////////////////////////////////////

bool cGuid::operator ==(const cGuid & other) const
{
   return memcmp(static_cast<const sGuid *>(this),
                 static_cast<const sGuid *>(&other),
                 sizeof(sGuid)) == 0;
}

///////////////////////////////////////

bool cGuid::operator !=(const cGuid & other) const
{
   return memcmp(static_cast<const sGuid *>(this),
                 static_cast<const sGuid *>(&other),
                 sizeof(sGuid)) != 0;
}

///////////////////////////////////////

bool cGuid::ToString(char * psz, int maxLen) const
{
   int result = snprintf(psz, maxLen - 1, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      Data1, Data2, Data3,
      Data4[0], Data4[1], Data4[2], Data4[3],
      Data4[4], Data4[5], Data4[6], Data4[7]);
   return result > 0;
}

///////////////////////////////////////////////////////////////////////////////

bool GUIDToString(REFGUID guid, char * psz, int maxLen)
{
   int result = snprintf(psz, maxLen - 1, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      guid.Data1, guid.Data2, guid.Data3,
      guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
      guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
   return result > 0;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cComToolsTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cComToolsTests);
      CPPUNIT_TEST(TestcGuidToString);
      CPPUNIT_TEST(TestIsSameObject);
   CPPUNIT_TEST_SUITE_END();

   void TestcGuidToString();
   void TestIsSameObject();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cComToolsTests);

////////////////////////////////////////

void cComToolsTests::TestcGuidToString()
{
   // {B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}
   static const cGuid kTestGuidA(0xb40b6831, 0xfcb2, 0x4082, 0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb);
   static const cGuid kTestGuidADupe(0xb40b6831, 0xfcb2, 0x4082, 0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb);

   // {DED26BAE-E83F-4c59-8B95-A309311B15A9}
   static const cGuid kTestGuidB(0xded26bae, 0xe83f, 0x4c59, 0x8b, 0x95, 0xa3, 0x9, 0x31, 0x1b, 0x15, 0xa9);

   char sz[kGuidStringLength];
   CPPUNIT_ASSERT(kTestGuidA.ToString(sz, _countof(sz)));
   CPPUNIT_ASSERT(strcmp(sz, "{B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}") == 0);

   char szTooSmall[kGuidStringLength / 4];
   CPPUNIT_ASSERT(!kTestGuidA.ToString(szTooSmall, _countof(szTooSmall)));

   CPPUNIT_ASSERT(kTestGuidA == kTestGuidADupe);
   CPPUNIT_ASSERT(kTestGuidA != kTestGuidB);
}

////////////////////////////////////////

GUID IID_ISameObjectTest1a = 
{ 0x4e4c5299, 0x18bd, 0x4477, { 0x9c, 0x4b, 0x2b, 0x40, 0xa6, 0x98, 0x82, 0x4b } };
GUID IID_ISameObjectTest1b = 
{ 0xdad942b, 0xd626, 0x4cfe, { 0x92, 0xe6, 0xca, 0xad, 0x88, 0xd3, 0xab, 0x29 } };
GUID IID_ISameObjectTest2 = 
{ 0x3eb94c5f, 0x2e2c, 0x4fbc, { 0xbe, 0x49, 0x1c, 0xc0, 0xc8, 0xad, 0x6f, 0xde } };

interface ISameObjectTest1a : IUnknown {};
interface ISameObjectTest1b : IUnknown {};
class cSameObjectTest1 : public cComObject2<IMPLEMENTS(ISameObjectTest1a), IMPLEMENTS(ISameObjectTest1b)> {};
interface ISameObjectTest2 : IUnknown {};
class cSameObjectTest2 : public cComObject<IMPLEMENTS(ISameObjectTest2)> {};

void cComToolsTests::TestIsSameObject()
{
   cAutoIPtr<cSameObjectTest1> pObj1(new cSameObjectTest1);

   cAutoIPtr<ISameObjectTest1a> pObj1a;
   CPPUNIT_ASSERT(pObj1->QueryInterface(IID_ISameObjectTest1a, (void**)&pObj1a) == S_OK);

   cAutoIPtr<ISameObjectTest1b> pObj1b;
   CPPUNIT_ASSERT(pObj1->QueryInterface(IID_ISameObjectTest1b, (void**)&pObj1b) == S_OK);

   CPPUNIT_ASSERT(CTIsSameObject(static_cast<ISameObjectTest1b*>(pObj1), pObj1a));
   CPPUNIT_ASSERT(CTIsSameObject(static_cast<ISameObjectTest1a*>(pObj1), pObj1b));
   CPPUNIT_ASSERT(CTIsSameObject(pObj1a, pObj1b));

   cAutoIPtr<cSameObjectTest2> pObj2(new cSameObjectTest2);

   CPPUNIT_ASSERT(!CTIsSameObject(static_cast<ISameObjectTest1a*>(pObj1), pObj2));
   CPPUNIT_ASSERT(!CTIsSameObject(pObj1a, pObj2));
   CPPUNIT_ASSERT(!CTIsSameObject(pObj1b, pObj2));

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
