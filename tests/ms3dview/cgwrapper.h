/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CGWRAPPER_H
#define INCLUDED_CGWRAPPER_H

#ifdef _MSC_VER
#pragma once
#endif

typedef struct _CGcontext * CGcontext;
typedef struct _CGprogram * CGprogram;
typedef struct _CGparameter * CGparameter;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCgContext
//

class cCgContext
{
public:
   cCgContext();
   ~cCgContext();

   CGprogram LoadProgram(HINSTANCE hInst, LPCSTR pResId);

private:
   static void CgErrorCallback();
   static bool LoadResourceText(HINSTANCE hInst, LPCSTR pResId, LPCSTR pResType, CString * pText);

   static CGcontext gm_cgContext;
   static ulong gm_cgContextRefs;
   long m_cgProfile;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CGWRAPPER_H
