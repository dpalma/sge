///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cpufeatures.h"

#include "techstring.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#ifdef _WIN32
#include <excpt.h>
#endif

#include <cstring>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(CpuFeatures);

#define LocalMsg(msg)            DebugMsgEx(CpuFeatures,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(CpuFeatures,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(CpuFeatures,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(CpuFeatures,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(CpuFeatures,(msg),(a),(b),(c),(d))

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

#ifdef HAVE_CPPUNITLITE2

TEST(TestCpuFeatures)
{
   static const struct
   {
      int featureFlag;
      const tChar * pszFeature;
   }
   featureFlagNames[] =
   {
      { kCpuHasRdtsc, _T("RDTSC") },
      { kCpuHasMmx, _T("MMX") },
      { kCpuHasSse, _T("SSE") },
      { kCpuHasSse2, _T("SSE2") },
      { kCpuHasHyperThreading, _T("HyperThreading") },
   };

   sCpuFeatures cpuFeatures;
   CHECK(GetCpuFeatures(&cpuFeatures));
   LocalMsg1("CPU Vendor:  %s\n", cpuFeatures.szVendor);
   LocalMsg1("CPU Model:   %s\n", cpuFeatures.szModel);
   std::string flags;
   for (int i = 0; i < _countof(featureFlagNames); i++)
   {
      int f = featureFlagNames[i].featureFlag;
      if ((cpuFeatures.features & f) == f)
      {
         if (i > 0)
         {
            flags += _T(", ");
         }
         flags += featureFlagNames[i].pszFeature;
      }
   }
   LocalMsg1("CPU Flags:   %s\n", flags.c_str());
   std::string brand(cpuFeatures.szBrand);
   TrimLeadingSpace(&brand);
   TrimTrailingSpace(&brand);
   LocalMsg1("CPU Brand:   %s\n", brand.c_str());
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
