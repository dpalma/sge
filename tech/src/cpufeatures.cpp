///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cpufeatures.h"

#ifdef _WIN32
#include <excpt.h>
#endif

#include <cstring>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
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
   char szBrand[kMaxBrandString];

   memset(szVendor, 0, sizeof(szVendor));
   memset(szBrand, 0, sizeof(szBrand));

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
      ; Get basic information
      mov eax, 1
      cpuid
      mov version, eax
      mov info, ebx
      mov features, edx
      bt eax, 80000000h
      jnc End
      ; Get extended information
      mov eax, 80000000h
      cpuid
      test eax, 800000004h
      jl End
      mov eax, 80000002h
      cpuid
      mov dword ptr [szBrand], eax
      mov dword ptr [szBrand+4], ebx
      mov dword ptr [szBrand+8], ecx
      mov dword ptr [szBrand+12], edx
      mov eax, 80000003h
      cpuid
      mov dword ptr [szBrand+16], eax
      mov dword ptr [szBrand+20], ebx
      mov dword ptr [szBrand+24], ecx
      mov dword ptr [szBrand+28], edx
      mov eax, 80000004h
      cpuid
      mov dword ptr [szBrand+32], eax
      mov dword ptr [szBrand+36], ebx
      mov dword ptr [szBrand+40], ecx
      mov dword ptr [szBrand+44], edx
End:
      pop ebx
      pop ecx
      pop edx
   }

   memset(pCpuFeatures, 0, sizeof(sCpuFeatures));

   szVendor[kMaxVendorName-1] = 0;
   memcpy(pCpuFeatures->szVendor, szVendor, kMaxVendorName * sizeof(char));

   pCpuFeatures->features = features;

   szBrand[kMaxBrandString-1] = 0;
   memcpy(pCpuFeatures->szBrand, szBrand, kMaxBrandString * sizeof(char));

   return true;
}
#else
bool GetCpuFeatures(sCpuFeatures * pCpuFeatures)
{
   Assert(pCpuFeatures != NULL);

   uint vs1, vs2, vs3;
   uint version, info, features;

   asm(
      "xor %%eax, %%eax\t\n"
      "cpuid\t\n"
      "mov %%ebx, %0\t\n"
      "mov %%edx, %1\t\n"
      "mov %%ecx, %2\t\n"
      "test $1, %%eax\t\n"
      "jl End\t\n"
      "mov $1, %%eax\t\n"
      "cpuid\t\n"
      "mov %%eax, %3\t\n"
      "mov %%ebx, %4\t\n"
      "mov %%edx, %5\t\n"
"End:\t\n"
      : "=m"(vs1), "=m"(vs2), "=m"(vs3), "=m"(version), "=m"(info), "=m"(features)
      :
      : "%ebx", "%ecx", "%edx");

   memset(pCpuFeatures, 0, sizeof(sCpuFeatures));

   memcpy(pCpuFeatures->szVendor, &vs1, 4);
   memcpy(pCpuFeatures->szVendor+4, &vs2, 4);
   memcpy(pCpuFeatures->szVendor+8, &vs3, 4);
   pCpuFeatures->szVendor[kMaxVendorName-1] = 0;

   pCpuFeatures->features = features;

   return true;
}
#endif

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
