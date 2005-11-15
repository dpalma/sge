///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "multivar.h"

#include "techmath.h"
#include "techstring.h"

#include <cstdio>
#include <cstdlib>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMultiVar
//

////////////////////////////////////////

cMultiVar::cMultiVar()
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Empty)
{
}

////////////////////////////////////////

cMultiVar::cMultiVar(const cMultiVar & other)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Empty)
{
   Assign(other);
}

////////////////////////////////////////

cMultiVar::cMultiVar(int i)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Int)
{
   m_value.i = i;
}

////////////////////////////////////////

cMultiVar::cMultiVar(float f)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Float)
{
   m_value.f = f;
}

////////////////////////////////////////

cMultiVar::cMultiVar(double d)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Double)
{
   m_value.d = d;
}

////////////////////////////////////////

cMultiVar::cMultiVar(const char * psz)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
{
   Assign(psz);
}

////////////////////////////////////////

cMultiVar::cMultiVar(const wchar_t * pwsz)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
{
   Assign(pwsz);
}

////////////////////////////////////////

cMultiVar::cMultiVar(IUnknown * pUnk)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Interface)
{
   m_value.pUnk = CTAddRef(pUnk);
}

////////////////////////////////////////

cMultiVar::~cMultiVar()
{
   Clear();
}

////////////////////////////////////////

void cMultiVar::Assign(const cMultiVar & multiVar)
{
   if (GetType() != multiVar.GetType())
   {
      Clear();
   }
   if (multiVar.IsInt())
   {
      Assign(multiVar.ToInt());
   }
   else if (multiVar.IsFloat())
   {
      Assign(multiVar.ToFloat());
   }
   else if (multiVar.IsDouble())
   {
      Assign(multiVar.ToDouble());
   }
   else if (multiVar.IsString())
   {
      Assign(multiVar.m_value.psz);
   }
   else if (multiVar.IsInterface())
   {
      m_type = kMVT_Interface;
      m_value.pUnk = CTAddRef(multiVar.m_value.pUnk);
   }
}

////////////////////////////////////////

void cMultiVar::Assign(int value)
{
   if (GetType() != kMVT_Int)
   {
      Clear();
      m_type = kMVT_Int;
   }
   m_value.i = value;
}

////////////////////////////////////////

void cMultiVar::Assign(float value)
{
   if (GetType() != kMVT_Float)
   {
      Clear();
      m_type = kMVT_Float;
   }
   m_value.f = value;
}

////////////////////////////////////////

void cMultiVar::Assign(double value)
{
   if (GetType() != kMVT_Double)
   {
      Clear();
      m_type = kMVT_Double;
   }
   m_value.d = value;
}

////////////////////////////////////////

void cMultiVar::Assign(const char * pszValue)
{
   if (GetType() != kMVT_String)
   {
      Clear();
      m_type = kMVT_String;
   }
   if (pszValue != NULL)
   {
      uint length = strlen(pszValue);
      uint bufferSize = (length + 1) * sizeof(char);
      m_pTempBuffer = realloc(m_pTempBuffer, bufferSize);
      m_value.psz = reinterpret_cast<char*>(m_pTempBuffer);
      strcpy(m_value.psz, pszValue);
   }
}

////////////////////////////////////////

void cMultiVar::Assign(const wchar_t * pwszValue)
{
   if (GetType() != kMVT_String)
   {
      Clear();
      m_type = kMVT_String;
   }
   if (pwszValue != NULL)
   {
      uint length = wcslen(pwszValue);
      uint bufferSize = (length + 1) * sizeof(wchar_t);
      m_pTempBuffer = realloc(m_pTempBuffer, bufferSize);
      m_value.psz = reinterpret_cast<char*>(m_pTempBuffer);
      wcstombs(m_value.psz, pwszValue, bufferSize);
   }
}

////////////////////////////////////////

void cMultiVar::Assign(IUnknown * pUnk)
{
   if (GetType() != kMVT_String)
   {
      Clear();
      m_type = kMVT_Interface;
   }
   m_value.pUnk = CTAddRef(pUnk);
}

////////////////////////////////////////

int cMultiVar::ToInt() const
{
   if (m_type == kMVT_Int)
   {
      return m_value.i;
   }
   else if (m_type == kMVT_Float)
   {
      return Round(m_value.f);
   }
   else if (m_type == kMVT_Double)
   {
      return Round(m_value.d);
   }
   else if (m_type == kMVT_String)
   {
      return Round(strtod(m_value.psz, NULL));
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as an integer\n");
      return 0;
   }
}

////////////////////////////////////////

float cMultiVar::ToFloat() const
{
   if (m_type == kMVT_Int)
   {
      return static_cast<float>(m_value.i);
   }
   else if (m_type == kMVT_Float)
   {
      return m_value.f;
   }
   else if (m_type == kMVT_Double)
   {
      return static_cast<float>(m_value.d);
   }
   else if (m_type == kMVT_String)
   {
      return static_cast<float>(strtod(m_value.psz, NULL));
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a float\n");
      return 0;
   }
}

////////////////////////////////////////

double cMultiVar::ToDouble() const
{
   if (m_type == kMVT_Int)
   {
      return static_cast<double>(m_value.i);
   }
   else if (m_type == kMVT_Float)
   {
      return static_cast<double>(m_value.f);
   }
   else if (m_type == kMVT_Double)
   {
      return m_value.d;
   }
   else if (m_type == kMVT_String)
   {
      return strtod(m_value.psz, NULL);
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a double\n");
      return 0;
   }
}

////////////////////////////////////////

const char * cMultiVar::ToString() const
{
   if (m_type == kMVT_Int)
   {
      if (m_pConversionBuffer == NULL)
      {
         cStr temp;
         temp.Format("%d", m_value.i);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(char);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
         strcpy(reinterpret_cast<char*>(m_pConversionBuffer), temp.c_str());
      }
      return reinterpret_cast<const char*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_Float || m_type == kMVT_Double)
   {
      if (m_pConversionBuffer == NULL)
      {
         cStr temp;
         temp.Format("%f", (m_type == kMVT_Float) ? m_value.f : m_value.d);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(char);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
         strcpy(reinterpret_cast<char*>(m_pConversionBuffer), temp.c_str());
      }
      return reinterpret_cast<const char*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_String)
   {
      return m_value.psz;
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a string\n");
      return NULL;
   }
}

////////////////////////////////////////

const wchar_t * cMultiVar::ToWideString() const
{
   if (m_type == kMVT_Int)
   {
      if (m_pConversionBuffer == NULL)
      {
         cStr temp;
         temp.Format("%d", m_value.i);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(char);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
#ifdef _UNICODE
         wcscpy(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str());
#else
         mbstowcs(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str(), bufferSize);
#endif
      }
      return reinterpret_cast<const wchar_t*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_Float || m_type == kMVT_Double)
   {
      if (m_pConversionBuffer == NULL)
      {
         cStr temp;
         temp.Format("%f", (m_type == kMVT_Float) ? m_value.f : m_value.d);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(wchar_t);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
#ifdef _UNICODE
         wcscpy(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str());
#else
         mbstowcs(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str(), bufferSize);
#endif
      }
      return reinterpret_cast<const wchar_t*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_String)
   {
      if (m_pConversionBuffer == NULL)
      {
         uint length = strlen(m_value.psz);
         uint bufferSize = (length + 1) * sizeof(wchar_t);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
         mbstowcs(reinterpret_cast<wchar_t*>(m_pConversionBuffer), m_value.psz, bufferSize);
      }
      return reinterpret_cast<const wchar_t*>(m_pConversionBuffer);
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a wide-character string\n");
      return NULL;
   }
}

////////////////////////////////////////

cMultiVar::operator IUnknown *() const
{
   if (m_type == kMVT_Interface)
   {
      return m_value.pUnk;
   }
   return NULL;
}

////////////////////////////////////////

void cMultiVar::Clear()
{
   if (m_type == kMVT_Interface)
   {
      SafeRelease(m_value.pUnk);
   }
   if (m_pTempBuffer != NULL)
   {
      free(m_pTempBuffer);
      m_pTempBuffer = NULL;
   }
   if (m_pConversionBuffer != NULL)
   {
      free(m_pConversionBuffer);
      m_pConversionBuffer = NULL;
   }
   m_type = kMVT_Empty;
   memset(&m_value, 0, sizeof(m_value));
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cMultiVarTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cMultiVarTests);
      CPPUNIT_TEST(TestConstructors);
      CPPUNIT_TEST(TestAssignment);
      CPPUNIT_TEST(TestCastDouble);
      CPPUNIT_TEST(TestCastString);
      CPPUNIT_TEST(TestCastWideCharString);
      CPPUNIT_TEST(TestChangeType);
   CPPUNIT_TEST_SUITE_END();

   void TestConstructors();
   void TestAssignment();
   void TestCastDouble();
   void TestCastString();
   void TestCastWideCharString();
   void TestChangeType();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cMultiVarTests);

////////////////////////////////////////

void cMultiVarTests::TestConstructors()
{
   {
      cMultiVar multiVarInt(9999);
      CPPUNIT_ASSERT(multiVarInt.GetType() == kMVT_Int);
      CPPUNIT_ASSERT(multiVarInt.IsInt());
      CPPUNIT_ASSERT(multiVarInt.ToInt() == 9999);
   }

   {
      cMultiVar multiVarFloat(kPi);
      CPPUNIT_ASSERT(multiVarFloat.GetType() == kMVT_Float);
      CPPUNIT_ASSERT(multiVarFloat.IsFloat());
      CPPUNIT_ASSERT(multiVarFloat.ToInt() == 3);
      CPPUNIT_ASSERT(multiVarFloat.ToFloat() == kPi);
   }

   {
      double d = 2 * 3.14159;
      cMultiVar multiVarDouble(d);
      CPPUNIT_ASSERT(multiVarDouble.GetType() == kMVT_Double);
      CPPUNIT_ASSERT(multiVarDouble.IsDouble());
      CPPUNIT_ASSERT(multiVarDouble.ToInt() == 6);
      CPPUNIT_ASSERT(multiVarDouble.ToDouble() == d);
   }

   {
      cMultiVar multiVarString("Single-byte string");
      CPPUNIT_ASSERT(multiVarString.GetType() == kMVT_String);
      CPPUNIT_ASSERT(multiVarString.IsString());
      CPPUNIT_ASSERT(strcmp(multiVarString.ToString(), "Single-byte string") == 0);
   }

   {
      cMultiVar multiVarWideString(L"Wide-character string");
      CPPUNIT_ASSERT(multiVarWideString.GetType() == kMVT_String);
      CPPUNIT_ASSERT(multiVarWideString.IsString());
      CPPUNIT_ASSERT(wcscmp(multiVarWideString.ToWideString(), L"Wide-character string") == 0);
   }

   {
      cMultiVar multiVarInterface(static_cast<IUnknown*>(NULL));
      CPPUNIT_ASSERT(multiVarInterface.GetType() == kMVT_Interface);
      CPPUNIT_ASSERT(multiVarInterface.IsInterface());
   }

   {
      cMultiVar multiVarEmpty;
      CPPUNIT_ASSERT(multiVarEmpty.GetType() == kMVT_Empty);
      CPPUNIT_ASSERT(multiVarEmpty.IsEmpty());
   }
}

////////////////////////////////////////

void cMultiVarTests::TestAssignment()
{
   cMultiVar test;
   CPPUNIT_ASSERT((double)(test = 3.1415) == 3.1415);
   CPPUNIT_ASSERT(strcmp((test = "3.1415"), "3.1415") == 0);
}

////////////////////////////////////////

void cMultiVarTests::TestCastDouble()
{
   CPPUNIT_ASSERT((double)cMultiVar(3.1415) == 3.1415);
   CPPUNIT_ASSERT((double)cMultiVar("3.1415") == 3.1415);
}

////////////////////////////////////////

void cMultiVarTests::TestCastString()
{
   CPPUNIT_ASSERT(strcmp((const char *)cMultiVar(3.1415), "3.141500") == 0);
   CPPUNIT_ASSERT(strcmp((const char *)cMultiVar("3.1415"), "3.1415") == 0);
}

////////////////////////////////////////

void cMultiVarTests::TestCastWideCharString()
{
   CPPUNIT_ASSERT(wcscmp((const wchar_t *)cMultiVar(3.1415), L"3.141500") == 0);
   CPPUNIT_ASSERT(wcscmp((const wchar_t *)cMultiVar("3.1415"), L"3.1415") == 0);
}

////////////////////////////////////////

void cMultiVarTests::TestChangeType()
{
   cMultiVar v(kPi);
   CPPUNIT_ASSERT(v.ToFloat() == kPi);
   v = L"3.141500";
   CPPUNIT_ASSERT(wcscmp((const wchar_t *)cMultiVar(3.1415), L"3.141500") == 0);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
