/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "cgwrapper.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "cg")
#pragma comment(lib, "cgGL")

//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCgContext
//

CGcontext cCgContext::gm_cgContext = NULL;
ulong cCgContext::gm_cgContextRefs = 0;

cCgContext::cCgContext()
 : m_cgProfile(CG_PROFILE_UNKNOWN)
{
   if (gm_cgContext == NULL)
   {
      gm_cgContext = cgCreateContext();
      Verify(++gm_cgContextRefs == 1);
   }
   else
   {
      ++gm_cgContextRefs;
   }

   cgSetErrorCallback(CgErrorCallback);
}

cCgContext::~cCgContext()
{
   if (gm_cgContextRefs > 0)
   {
      if (--gm_cgContextRefs == 0)
      {
         if (gm_cgContext != NULL)
         {
            cgDestroyContext(gm_cgContext);
            gm_cgContext = NULL;
         }
      }
   }
}

CGprogram cCgContext::LoadProgram(HINSTANCE hInst, LPCSTR pResId)
{
   if (m_cgProfile == CG_PROFILE_UNKNOWN)
   {
      m_cgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
      if (m_cgProfile == CG_PROFILE_UNKNOWN)
      {
         return NULL;
      }
   }

   CString cgCode;
   if (LoadResourceText(hInst, pResId, "CG", &cgCode))
   {
      CGprogram program = cgCreateProgram(gm_cgContext, CG_SOURCE, cgCode,
         (CGprofile)m_cgProfile, NULL, NULL);

      if (program != NULL)
      {
         cgGLLoadProgram(program);
         return program;
      }
   }

   return NULL;
}

void cCgContext::CgErrorCallback()
{
   CGerror lastError = cgGetError();

   if (lastError)
   {
      DebugMsg(cgGetErrorString(lastError));

      const char * pszListing = cgGetLastListing(gm_cgContext);
      if (pszListing != NULL)
      {
         DebugMsg1("   %s\n", pszListing);
      }
   }
}

bool cCgContext::LoadResourceText(HINSTANCE hInst, LPCSTR pResId, LPCSTR pResType, CString * pText)
{
   HRSRC hR = FindResource(hInst, pResId, pResType);
   if (hR != NULL)
   {
      uint resSize = SizeofResource(hInst, hR);
      HGLOBAL hG = LoadResource(hInst, hR);
      if (hG != NULL)
      {
         void * pResData = LockResource(hG);
         if (pResData)
         {
            *pText = (char *)pResData;
            return true;
         }
      }
   }
   return false;
}
