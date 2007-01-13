/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_DYNAMICLINK_H)
#define INCLUDED_DYNAMICLINK_H

#include <string>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_HANDLE(HINSTANCE);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDynamicLink
//

class cDynamicLink
{
public:
   cDynamicLink(const tChar * pszLibrary);
   ~cDynamicLink();

   FARPROC GetProcAddress(const tChar * pszProc);

private:
   std::string m_name;
   HINSTANCE m_hLibrary;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_DYNAMICLINK_H)
