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

cScriptVar::cScriptVar(const cScriptVar & other)
{
   operator =(other);
}

///////////////////////////////////////

const cScriptVar & cScriptVar::operator =(double _d)
{
   type = kNumber;
   d = _d;
   return *this;
}

///////////////////////////////////////

const cScriptVar & cScriptVar::operator =(char * _psz)
{
   type = kString;
   psz = _psz;
   return *this;
}

///////////////////////////////////////

const cScriptVar & cScriptVar::operator =(const cScriptVar & other)
{
   type = other.type;
   if (type == kString)
      psz = other.psz;
   else
      d = other.d;
   return *this;
}

///////////////////////////////////////

cScriptVar::operator double() const
{
   if (type == kString)
   {
      return strtod(psz, NULL);
   }
   else
   {
      return d;
   }
}

///////////////////////////////////////

cScriptVar::operator const char *() const
{
   static int index = 0;
   static char szBuffers[50][4];
   if (type == kNumber)
   {
      index = (index + 1) & 3;
      sprintf(szBuffers[index], "%f", d);
      return szBuffers[index];
   }
   else
   {
      return psz;
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

private:
   void TestAssignment()
   {
      cScriptVar test;
      CPPUNIT_ASSERT((double)(test = 3.1415) == 3.1415);
      CPPUNIT_ASSERT(strcmp((test = "3.1415"), "3.1415") == 0);
   }

   void TestCastDouble()
   {
      CPPUNIT_ASSERT((double)cScriptVar(3.1415) == 3.1415);
      CPPUNIT_ASSERT((double)cScriptVar("3.1415") == 3.1415);
   }

   void TestCastString()
   {
      CPPUNIT_ASSERT(strcmp((const char *)cScriptVar(3.1415), "3.141500") == 0);
      CPPUNIT_ASSERT(strcmp((const char *)cScriptVar("3.1415"), "3.1415") == 0);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cScriptVarTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
