///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifndef _WIN32
tResult DictionaryStoreCreate(HKEY, const tChar *, bool, IDictionaryStore * *)
{
   return E_NOTIMPL;
}
#else

#include "dictregstore.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionaryRegistryStore
//

///////////////////////////////////////

cDictionaryRegistryStore::cDictionaryRegistryStore(HKEY hKey, bool bTakeOwnership)
 : m_hKey(hKey)
 , m_bCloseKeyOnDestroy(bTakeOwnership)
{
}

///////////////////////////////////////

cDictionaryRegistryStore::~cDictionaryRegistryStore()
{
   if (m_bCloseKeyOnDestroy && m_hKey != NULL)
   {
      RegCloseKey(m_hKey);
   }
   m_hKey = NULL;
}

///////////////////////////////////////

tResult cDictionaryRegistryStore::Load(IDictionary * pDictionary)
{
   if (pDictionary == NULL)
   {
      return E_POINTER;
   }

   Assert(m_hKey != NULL);

   int nValues = 0;

   for (DWORD iValue = 0; ; iValue++)
   {
      tChar szName[256];
      DWORD dwNameLen = _countof(szName);
      DWORD dwType = 0;
      DWORD dwValueLen = 0;
      LONG result = RegEnumValue(m_hKey, iValue, szName, &dwNameLen, NULL, &dwType, NULL, &dwValueLen);
      if (result == ERROR_NO_MORE_ITEMS)
      {
         break;
      }
      else if (result != ERROR_SUCCESS)
      {
         ErrorMsg1("RegEnumValue returned error code %d\n", result);
         return E_FAIL;
      }

      if (dwType == REG_SZ)
      {
         byte * pTemp = reinterpret_cast<byte*>(alloca(dwValueLen + 1));
         if (RegEnumValue(m_hKey, iValue, szName, &dwNameLen, NULL, &dwType, pTemp, &dwValueLen) == ERROR_SUCCESS)
         {
            if (pDictionary->Set(szName, reinterpret_cast<tChar*>(pTemp)) == S_OK)
            {
               nValues++;
            }
         }
      }
      else if (dwType == REG_DWORD && dwValueLen == sizeof(DWORD))
      {
         DWORD dwValue = 0;
         if (RegEnumValue(m_hKey, iValue, szName, &dwNameLen, NULL, &dwType, reinterpret_cast<byte*>(&dwValue), &dwValueLen) == ERROR_SUCCESS)
         {
            if (pDictionary->Set(szName, static_cast<int>(dwValue)) == S_OK)
            {
               nValues++;
            }
         }
      }
      else
      {
         WarnMsg1("Skipping registry value of type %x\n", dwType);
      }
   }

   return (nValues == 0) ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cDictionaryRegistryStore::Save(IDictionary * pDictionary)
{
   return E_NOTIMPL; // TODO
}

///////////////////////////////////////

tResult cDictionaryRegistryStore::MergeSave(IDictionary * pDictionary)
{
   return E_NOTIMPL; // TODO
}

///////////////////////////////////////

tResult DictionaryStoreCreate(HKEY hKey, const tChar * pszSubKey, bool bReadOnly, IDictionaryStore * * ppStore)
{
   if (hKey == NULL || pszSubKey == NULL || ppStore == NULL)
   {
      return E_POINTER;
   }

   DWORD dwResult = 0;
   HKEY hStoreKey = NULL;
   if ((dwResult = RegOpenKeyEx(hKey, pszSubKey, 0, bReadOnly ? KEY_READ : KEY_ALL_ACCESS, &hStoreKey)) != ERROR_SUCCESS)
   {
      ErrorMsg1("RegOpenKeyEx failed with error %d\n", dwResult);
      return E_FAIL;
   }

   cAutoIPtr<IDictionaryStore> pStore(new cDictionaryRegistryStore(hStoreKey));

   if (!pStore)
   {
      RegCloseKey(hStoreKey);
      return E_OUTOFMEMORY;
   }

   return pStore.GetPointer(ppStore);
}


#endif // _WIN32 (entire file)

///////////////////////////////////////////////////////////////////////////////
