///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "scriptvar.h"

#include <cstdio>
#include <cstdlib>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptVar
//

///////////////////////////////////////

cScriptVar::cScriptVar()
{
   type = kEmpty;
}

///////////////////////////////////////

cScriptVar::cScriptVar(double _d)
{
   type = kNumber;
   d = _d;
}

///////////////////////////////////////

cScriptVar::cScriptVar(char * _psz)
{
   type = kString;
   psz = _psz;
}

///////////////////////////////////////

cScriptVar::cScriptVar(IUnknown * _pUnk)
{
   type = kInterface;
   pUnk = CTAddRef(_pUnk);
}

///////////////////////////////////////

cScriptVar::cScriptVar(const cScriptVar & other)
{
   operator =(other);
}

///////////////////////////////////////

cScriptVar::~cScriptVar()
{
   Clear();
}

///////////////////////////////////////

const cScriptVar & cScriptVar::operator =(double _d)
{
   Clear();
   type = kNumber;
   d = _d;
   return *this;
}

///////////////////////////////////////

const cScriptVar & cScriptVar::operator =(char * _psz)
{
   Clear();
   type = kString;
   psz = _psz;
   return *this;
}

///////////////////////////////////////

const cScriptVar & cScriptVar::operator =(IUnknown * _pUnk)
{
   Clear();
   type = kInterface;
   pUnk = CTAddRef(_pUnk);
   return *this;
}

///////////////////////////////////////

const cScriptVar & cScriptVar::operator =(const cScriptVar & other)
{
   Clear();
   type = other.type;
   if (type == kString)
   {
      psz = other.psz;
   }
   else if (type == kNumber)
   {
      d = other.d;
   }
   else if (type = kInterface)
   {
      pUnk = CTAddRef(other.pUnk);
   }
   return *this;
}

///////////////////////////////////////

cScriptVar::operator double() const
{
   if (type == kString)
   {
      return strtod(psz, NULL);
   }
   else if (type == kNumber)
   {
      return d;
   }
   else
   {
      DebugMsg("WARNING: attempt to access %d scriptvar as double\n");
      return 0;
   }
}

///////////////////////////////////////

cScriptVar::operator const char *() const
{
   static const char kBufferSize = 50;
   static int index = 0;
   static char szBuffers[kBufferSize][4];
   if (type == kNumber)
   {
      index = (index + 1) & 3;
      snprintf(szBuffers[index], kBufferSize, "%f", d);
      return szBuffers[index];
   }
   else if (type == kString)
   {
      return psz;
   }
   else
   {
      DebugMsg("WARNING: attempt to access %d scriptvar as string\n");
      return NULL;
   }
}

///////////////////////////////////////

cScriptVar::operator IUnknown *() const
{
   if (type == kInterface)
   {
      return pUnk;
   }
   return NULL;
}

///////////////////////////////////////

void cScriptVar::Clear()
{
   if (type == kInterface)
   {
      SafeRelease(pUnk);
   }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cScriptVarTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cScriptVarTests);
      CPPUNIT_TEST(TestAssignment);
      CPPUNIT_TEST(TestCastDouble);
      CPPUNIT_TEST(TestCastString);
   CPPUNIT_TEST_SUITE_END();

   void TestAssignment();
   void TestCastDouble();
   void TestCastString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cScriptVarTests);

void cScriptVarTests::TestAssignment()
{
   cScriptVar test;
   CPPUNIT_ASSERT((double)(test = 3.1415) == 3.1415);
   CPPUNIT_ASSERT(strcmp((test = "3.1415"), "3.1415") == 0);
}

void cScriptVarTests::TestCastDouble()
{
   CPPUNIT_ASSERT((double)cScriptVar(3.1415) == 3.1415);
   CPPUNIT_ASSERT((double)cScriptVar("3.1415") == 3.1415);
}

void cScriptVarTests::TestCastString()
{
   CPPUNIT_ASSERT(strcmp((const char *)cScriptVar(3.1415), "3.141500") == 0);
   CPPUNIT_ASSERT(strcmp((const char *)cScriptVar("3.1415"), "3.1415") == 0);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
