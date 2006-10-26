///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DLL_H
#define INCLUDED_DLL_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDLL
//

class cDLL
{
public:
   cDLL() : m_hModule(NULL)
   {
   }

   ~cDLL()
   {
      Free();
   }

   bool Load(const tChar * pszDLL)
   {
      if (m_hModule == NULL)
      {
         m_hModule = LoadLibrary(pszDLL);
         return (m_hModule != NULL);
      }
      WarnMsg("Attempting to load cDLL object with non-NULL module handle\n");
      return false;
   }
   
   bool Free()
   {
      if (FreeLibrary(m_hModule))
      {
         m_hModule = NULL;
         return true;
      }
      return false;
   }

   bool IsLoaded() const
   {
      return (m_hModule != NULL);
   }

   FARPROC GetProcAddress(LPCSTR lpProcName)
   {
      if (m_hModule != NULL)
      {
         return ::GetProcAddress(m_hModule, lpProcName);
      }
      return NULL;
   }

private:
   HMODULE m_hModule;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_DLL_H
