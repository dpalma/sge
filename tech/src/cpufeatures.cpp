///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cpufeatures.h"

#ifdef _WIN32
#include <excpt.h>
#endif

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool GetCpuFeatures(sCpuFeatures * pCpuFeatures)
{
   Assert(pCpuFeatures != NULL);

   __try
   {
      __asm
      {
         xor eax, eax
         cpuid
      }
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      return false;
   }

   char szVendor[kMaxVendorName];
   uint version, info, features;

   __asm
   {
      push ebx
      push ecx
      push edx
      xor eax, eax
      cpuid
      mov dword ptr [szVendor], ebx
      mov dword ptr [szVendor+4], edx
      mov dword ptr [szVendor+8], ecx
      test eax, 1
      jl End 
      mov eax, 1
      cpuid
      mov version, eax
      mov info, ebx
      mov features, edx
End:
      pop ebx
      pop ecx
      pop edx
   }

   memset(pCpuFeatures, 0, sizeof(sCpuFeatures));

   szVendor[kMaxVendorName-1] = 0;
   memcpy(pCpuFeatures->szVendor, szVendor, kMaxVendorName * sizeof(char));

   pCpuFeatures->features = features;

   return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cCpuFeaturesTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cCpuFeaturesTests);
   CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cCpuFeaturesTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
